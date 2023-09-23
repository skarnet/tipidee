/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>

#include <skalibs/gccattributes.h>
#include <skalibs/posixplz.h>
#include <skalibs/types.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/error.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/cspawn.h>
#include <skalibs/djbunix.h>
#include <skalibs/iopause.h>
#include <skalibs/env.h>
#include <skalibs/exec.h>
#include <skalibs/unix-timed.h>
#include <skalibs/lolstdio.h>

#include <tipidee/method.h>
#include <tipidee/headers.h>
#include <tipidee/response.h>
#include <tipidee/uri.h>
#include "tipideed-internal.h"

static void addenv_ (tipidee_rql const *rql, char const *k, char const *v, int slash)
{
  if (!stralloc_cats(&g.sa, k)
   || !stralloc_catb(&g.sa, "=/", 1 + !!slash)
   || !stralloc_cats(&g.sa, v)
   || !stralloc_0(&g.sa))
    die500sys(rql, 111, "stralloc_catb") ;
}

#define addenv(rql, k, v) addenv_(rql, k, (v), 0)
#define addenvslash(rql, k, v) addenv_(rql, k, (v), 1)

static void delenv (tipidee_rql const *rql, char const *k)
{
  if (!stralloc_cats(&g.sa, k)
   || !stralloc_0(&g.sa))
    die500sys(rql, 111, "stralloc_catb") ;
}

static inline void modify_env (tipidee_rql const *rql, tipidee_headers const *hdr, size_t cl, char const *script, char const *infopath)
{
  uint32_t got = 0 ;
  addenv(rql, "REQUEST_METHOD", tipidee_method_tostr(rql->m)) ;
  if (cl)
  {
    char fmt[SIZE_FMT] ;
    fmt[size_fmt(fmt, cl)] = 0 ;
    addenv(rql, "CONTENT_LENGTH", fmt) ;
  }
  else delenv(rql, "CONTENT_LENGTH") ;

  if (infopath) addenvslash(rql, "PATH_INFO", infopath) ;
  else delenv(rql, "PATH_INFO") ;
  if (rql->uri.query) addenv(rql, "QUERY_STRING", rql->uri.query) ;
  else delenv(rql, "QUERY_STRING") ;
  addenv(rql, "SCRIPT_NAME", script) ;
  
  for (size_t i = 0 ; i < hdr->n ; i++)
  {
    char const *key = hdr->buf + hdr->list[i].left ;
    char const *val = hdr->buf + hdr->list[i].right ;
    if (!strcasecmp(key, "Authorization"))
    {
      size_t n = str_chr(val, ' ') ;
      if (n)
      {
        char scheme[n] ;
        memcpy(scheme, val, n-1) ;
        scheme[n-1] = 0 ;
        addenv(rql, "AUTH_TYPE", scheme) ;
        got |= 1 ;
      }
    }
    else if (!strcasecmp(key, "Content-Type")) { addenv(rql, "CONTENT_TYPE", val) ; got |= 2 ; }
    else if (!strcasecmp(key, "Content-Length") || !strcasecmp(key, "Connection")) ;
    else
    {
      size_t len = strlen(key), pos = g.sa.len + 5 ;
      if (!stralloc_catb(&g.sa, "HTTP_", 5)) die500sys(rql, 111, "stralloc_catb") ;
      addenv(rql, key, val) ;
      for (char *s = g.sa.s + pos ; len-- ; s++)
        if (*s == '-') *s = '_' ;
        else if (*s >= 'a' && *s <= 'z') *s -= 32 ;
    }
  }
  if (!(got & 1)) delenv(rql, "AUTH_TYPE") ;
  if (!(got & 2)) delenv(rql, "CONTENT_TYPE") ;
}

static inline int do_nph (tipidee_rql const *rql, char const *const *argv, char const *const *envp, char const *body, size_t bodylen) gccattr_noreturn ;
static inline int do_nph (tipidee_rql const *rql, char const *const *argv, char const *const *envp, char const *body, size_t bodylen)
{
  int p[2] ;
  if (pipe(p) == -1) die500sys(rql, 111, "pipe") ;
  if (bodylen)
  {
    switch (fork())
    {
      case -1 : die500sys(rql, 111, "fork") ;
      case 0 :      
      {
#define NAME "tipideed (nph helper for pid "
        tain deadline ;
        char buf[4096] ;
        buffer b = BUFFER_INIT(&buffer_write, p[1], buf, 4096) ;
        size_t m = sizeof(NAME) - 1 ;
        char progstr[sizeof(NAME) + PID_FMT] ;
        memcpy(progstr, NAME, m) ;
        m += pid_fmt(progstr + m, getppid()) ;
        progstr[m++] = ')' ; progstr[m++] = 0 ;
        PROG = progstr ;
        tain_add_g(&deadline, &g.cgitto) ;
        close(p[0]) ;
        if (ndelay_on(p[1]) == -1) strerr_diefu1sys(111, "set fd nonblocking") ;
        if (buffer_timed_put_g(&b, body, bodylen, &deadline) < bodylen
         || !buffer_timed_flush_g(&b, &deadline))
          strerr_diefu2sys(111, "write request body to nph ", argv[0]) ;
        _exit(0) ;
      }
      default : break ;
    }
  }
  close(p[1]) ;
  if (fd_move(0, p[0]) == -1) die500sys(rql, 111, "fd_move") ;
  log_nph(argv, envp) ;
  exec_e(argv, envp) ;
  die500sys(rql, errno == ENOENT ? 127 : 126, "exec nph ", argv[0]) ;
}

static inline int run_cgi (tipidee_rql const *rql, char const *const *argv, char const *const *envp, char const *body, size_t bodylen, tipidee_headers *hdr, stralloc *sa)
{
  iopause_fd x[2] = { { .events = IOPAUSE_READ }, { .events = IOPAUSE_WRITE } } ;
  size_t bodyw = 0 ;
  unsigned int rstate = 0 ;
  tain deadline ;
  pid_t pid ;
  disize curheader = DISIZE_ZERO ;
  uint32_t parserstate = 0 ;
  buffer b ;
  char buf[4096] ;
  log_cgi(argv, envp) ;
  {
    int fd[2] = { 0, 1 } ;
    pid = child_spawn2(argv[0], argv, envp, fd) ;
    if (!pid) die500sys(rql, 111, "spawn ", argv[0]) ;
    x[0].fd = fd[0] ; x[1].fd = fd[1] ;
  }
  if (!bodylen)
  {
    close(x[1].fd) ;
    x[1].fd = -1 ;
    LOLDEBUG("run_cgi: no request body, closing writing pipe to cgi") ;
  }
  buffer_init(&b, &buffer_read, x[0].fd, buf, 4096) ;
  tain_add_g(&deadline, &g.cgitto) ;
  while (x[0].fd >= 0)
  {
    int r = iopause_g(x, 1 + (x[1].fd >= 0), &deadline) ;
    if (r == -1) die500sys(rql, 111, "iopause") ;
    if (!r)
    {
      kill(pid, SIGTERM) ;
      respond_504(rql) ;
      if (g.verbosity >= 2) strerr_warnw3x("cgi ", argv[0], " timed out") ;
      break ;
    }
    if (x[1].fd >= 0 && x[1].revents & (IOPAUSE_WRITE | IOPAUSE_EXCEPT))
    {
      size_t len = allwrite(x[1].fd, body + bodyw, bodylen - bodyw) ;
      if (!len)
      {
        if (g.verbosity) strerr_warnwu2sys("write request body to cgi ", argv[0]) ;
        bodyw = bodylen ;
      }
      else bodyw += len ;
      if (bodyw >= bodylen)
      {
        close(x[1].fd) ;
        x[1].fd = -1 ;
        LOLDEBUG("run_cgi: finished writing body") ;
      }
    }
    if (x[0].fd >= 0 && x[0].revents & (IOPAUSE_READ | IOPAUSE_EXCEPT))
    {
      switch (rstate)
      {
        case 0 :
        {
          r = tipidee_headers_parse_nb(&b, hdr, &curheader, &parserstate) ;
          switch (r)
          {
            case -2 : break ;
            case -1 : die500sys(rql, 111, "read from cgi ", argv[0]) ;
            case 0 :
            {
              size_t n = buffer_len(&b) ;
              if (!stralloc_readyplus(sa, n)) die500sys(rql, 111, "stralloc_readyplus") ;
              buffer_getnofill(&b, sa->s + sa->len, n) ;
              sa->len += n ;
              rstate = 1 ;
              break ;
            }
            case 400 : die502x(rql, 2, "invalid headers", " from cgi ", argv[0]) ;
            case 413 : die502x(rql, 2, hdr->n >= TIPIDEE_HEADERS_MAX ? "Too many headers" : "Too much header data", " from cgi ", argv[0]) ;
            case 500 : die500x(rql, 101, "can't happen: ", "avltreen_insert failed", " in do_cgi") ;
            default : die500x(rql, 101, "can't happen: ", "unknown tipidee_headers_parse return code", " in do_cgi") ;
          }
          if (!rstate) break ;
        }
        case 1 :
        {
          if (!slurpn(x[0].fd, sa, g.maxcgibody))
          {
            if (error_isagain(errno)) break ;
            else if (errno == ENOBUFS) die502x(rql, 2, "Too fat body", " from cgi ", argv[0]) ;
            else die500sys(rql, 111, "read body", " from cgi ", argv[0]) ;
          }
          close(x[0].fd) ;
          x[0].fd = -1 ;
          rstate = 2 ;
          LOLDEBUG("run_cgi: rstate = 2") ;
        }
      }
    }
  }
  if (x[1].fd >= 0) close(x[1].fd) ;
  if (x[0].fd >= 0) close(x[0].fd) ;
  return rstate == 2 ;
}

static inline int local_redirect (tipidee_rql *rql, char const *loc, char *uribuf, char const *cginame)
{
  size_t n ;
  size_t hostlen = strlen(rql->uri.host) ;
  uint16_t port = rql->uri.port ;
  uint8_t ishttps = rql->uri.https ;
  char hosttmp[hostlen + 1] ;
  memcpy(hosttmp, rql->uri.host, hostlen + 1) ;
  n = tipidee_uri_parse(uribuf, URI_BUFSIZE, loc, &rql->uri) ;
  if (!n || n + hostlen + 1 > URI_BUFSIZE)
    die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Location", " value", " for local redirection") ;
  memcpy(uribuf + n, hosttmp, hostlen + 1) ;
  rql->uri.host = uribuf + n ;
  rql->uri.port = port ;
  rql->uri.https = ishttps ;
  if (g.verbosity >= 4) strerr_warni4x("cgi ", cginame, ": local redirect to ", rql->uri.path) ;
  return 1 ;
}

static inline void print_cgi_headers (tipidee_headers const *hdr, size_t rbodylen)
{
  static char const *const nope_table[] =
  {
    "Connection",
    "Date",
    "Status",
    "Content-Length",
    0
  } ;
  for (size_t i = 0 ; i < hdr->n ; i++)
  {
    char const *key = hdr->buf + hdr->list[i].left ;
    char const *const *p = nope_table ;
    if (tipidee_response_header_builtin_search(key)) continue ;
    if (str_start(key, "X-CGI-")) continue ;
    for (; *p ; p++) if (!strcasecmp(key, *p)) break ;
    if (*p) continue ;
    buffer_putsnoflush(buffer_1, key) ;
    buffer_putnoflush(buffer_1, ": ", 2) ;
    buffer_putsnoflush(buffer_1, hdr->buf + hdr->list[i].right) ;
    buffer_putnoflush(buffer_1, "\r\n", 2) ;
  }
  if (rbodylen)
  {
    char fmt[SIZE_FMT] ;
    fmt[size_fmt(fmt, rbodylen)] = 0 ;
    buffer_putsnoflush(buffer_1, "Content-Length: ") ;
    buffer_putsnoflush(buffer_1, fmt) ;
    buffer_putnoflush(buffer_1, "\r\n", 2) ;
  }
}

static inline int process_cgi_output (tipidee_rql *rql, tipidee_headers const *hdr, char const *rbody, size_t rbodylen, char *uribuf, char const *cginame)
{
  char const *location = tipidee_headers_search(hdr, "Location") ;
  char const *x = tipidee_headers_search(hdr, "Status") ;
  char const *reason_phrase = "OK" ;
  unsigned int status = 0 ;
  tain deadline ;
  tain_add_g(&deadline, &g.writetto) ;
  if (x)
  {
    size_t m = uint_scan(x, &status) ;
    if (!m || (x[m] && x[m] != ' '))
      die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Status", " header") ;
    reason_phrase = x[m] ? x + m + 1 : "" ;
    if (!location && (status == 301 || status == 302 || status == 307 || status == 308))
      die502x(rql, 2, "cgi ", cginame, " returned a redirection status code without a ", "Location", " header") ;
    if (status < 100 || status > 999)
      die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Status", " value") ;
  }
  if (location)
  {
    if (!location[0]) die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Location", " header") ;
    if (location[0] == '/' && location[1] != '/') return local_redirect(rql, location, uribuf, cginame) ;
    if (rbodylen)
    {
      if (!status)
        die502x(rql, 2, "cgi ", cginame, " didn't output a ", "Status", " header", " for a client redirect response with document") ;
      if (status < 300 || status > 399)
        die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Status", " value", " for a client redirect response with document") ;
    }
    else
    {
      for (size_t i = 0 ; i < hdr->n ; i++)
      {
        char const *key = hdr->buf + hdr->list[i].left ;
        if (!strcasecmp(key, "Location") || !strcasecmp(key, "Status")) continue ;
        if (str_start(key, "X-CGI-")) continue ;
        die502x(rql, 2, "cgi ", cginame, " returned extra headers", " for a client redirect response without document") ;
      }
      if (!status)
      {
        status = 302 ;
        reason_phrase = "Found" ;
      }
    }
  }
  else
  {
    if (!status) status = 200 ;
    if (!tipidee_headers_search(hdr, "Content-Type"))
      die502x(rql, 2, "cgi ", cginame, " didn't output a ", "Content-Type", " header") ;
  }
  x = tipidee_headers_search(hdr, "Content-Length") ;
  if (x)
  {
    size_t cln ;
    if (!size0_scan(x, &cln))
      die502x(rql, 2, "cgi ", cginame, " returned an invalid ", "Content-Length", " header") ;
    if (cln != rbodylen)
      die502x(rql, 2, "cgi ", cginame, " returned a mismatching ", "Content-Length", " header") ;
  }

  tipidee_response_status(buffer_1, rql, status, reason_phrase) ;
  tipidee_response_header_common_put_g(buffer_1, !g.cont) ;
  print_cgi_headers(hdr, rbodylen) ;
  if (buffer_timed_put_g(buffer_1, "\r\n", 2, &deadline) < 2)
    strerr_diefu1sys(111, "write to stdout") ;
  if (rbodylen)
  {
    if (buffer_timed_put_g(buffer_1, rbody, rbodylen, &deadline) < rbodylen)
      strerr_diefu1sys(111, "write to stdout") ;
  }
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  if (g.verbosity >= 2)
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, status)] = 0 ;
    if (status >= 300 && status < 399)
      strerr_warni6x("cgi ", cginame, ": ", fmt, " client redirect to ", location) ;
    else
      strerr_warni6x("cgi ", cginame, ": ", fmt, " ", reason_phrase) ;
  }
  return 0 ;
}

static inline int do_cgi (tipidee_rql *rql, char const *const *argv, char const *const *envp, char const *body, size_t bodylen, char *uribuf)
{
  static stralloc sa = STRALLOC_ZERO ;
  tipidee_headers hdr ;
  char hdrbuf[2048] ;
  sa.len = 0 ;
  tipidee_headers_init(&hdr, hdrbuf, 2048) ;
  if (!run_cgi(rql, argv, envp, body, bodylen, &hdr, &sa)) return 0 ;
  return process_cgi_output(rql, &hdr, sa.s, sa.len, uribuf, argv[0]) ;
}

int respond_cgi (tipidee_rql *rql, char const *fn, size_t docrootlen, char const *infopath, char *uribuf, tipidee_headers const *hdr, tipidee_resattr const *ra, char const *body, size_t bodylen)
{
  size_t sabase = g.sa.len ;
  size_t envmax = g.envlen + 16 + TIPIDEE_HEADERS_MAX ;
  char const *argv[2] = { fn, 0 } ;
  char const *envp[envmax] ;
  modify_env(rql, hdr, bodylen, fn + docrootlen, infopath) ;
  env_merge(envp, envmax, (char const *const *)environ, g.envlen, g.sa.s + g.cwdlen + 1, g.sa.len - (g.cwdlen+1)) ;
  g.sa.len = sabase ;
  return ra->isnph ? do_nph(rql, argv, envp, body, bodylen) :
                     do_cgi(rql, argv, envp, body, bodylen, uribuf) ;
}
