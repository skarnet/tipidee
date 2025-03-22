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
#include <skalibs/siovec.h>
#include <skalibs/unix-timed.h>

#include <tipidee/tipidee.h>
#include "tipideed-internal.h"

#define dienomem(rql, docroot) die500sys(rql, 111, (docroot), "stralloc_catb")

static void addenv_ (tipidee_rql const *rql, char const *docroot, char const *k, char const *v, size_t vlen, int slash)
{
  if (!stralloc_cats(&g.sa, k)
   || !stralloc_catb(&g.sa, "=/", 1 + !!slash)
   || !stralloc_catb(&g.sa, v, vlen)
   || !stralloc_0(&g.sa)) dienomem(rql, docroot) ;
}

#define addenv(rql, d, k, v) addenv_(rql, d, k, (v), strlen(v), 0)
#define addenvb(rql, d, k, v, vlen) addenv_(rql, d, k, v, (vlen), 0)
#define addenvslash(rql, d, k, v) addenv_(rql, d, k, (v), strlen(v), 1)

static void delenv (tipidee_rql const *rql, char const *docroot, char const *k)
{
  if (!stralloc_cats(&g.sa, k)
   || !stralloc_0(&g.sa)) dienomem(rql, docroot) ;
}

static void addrequesturi (tipidee_rql const *rql, char const *docroot, char const *script, char const *infopath)
{
  if (!stralloc_cats(&g.sa, "REQUEST_URI=")
   || !stralloc_cats(&g.sa, script)) dienomem(rql, docroot) ;
  if (infopath[0])
  {
    if (!stralloc_catb(&g.sa, "/", 1)
     || !stralloc_cats(&g.sa, infopath)) dienomem(rql, docroot) ;
  }
  if (rql->uri.query)
  {
    if (!stralloc_catb(&g.sa, "?", 1)
     || !stralloc_cats(&g.sa, rql->uri.query)) dienomem(rql, docroot) ;
  }
  if (!stralloc_0(&g.sa)) dienomem(rql, docroot) ;
}

static inline void modify_env (tipidee_rql const *rql, char const *docroot, tipidee_headers const *hdr, size_t cl, char const *script, char const *infopath)
{
  uint32_t got = 0 ;
  addenv(rql, docroot, "REQUEST_METHOD", tipidee_method_tostr(rql->m)) ;
  if (cl)
  {
    char fmt[SIZE_FMT] ;
    fmt[size_fmt(fmt, cl)] = 0 ;
    addenv(rql, docroot, "CONTENT_LENGTH", fmt) ;
  }
  else delenv(rql, docroot, "CONTENT_LENGTH") ;

  if (infopath[0]) addenvslash(rql, docroot, "PATH_INFO", infopath) ;
  else delenv(rql, docroot, "PATH_INFO") ;
  if (rql->uri.query) addenv(rql, docroot, "QUERY_STRING", rql->uri.query) ;
  else delenv(rql, docroot, "QUERY_STRING") ;
  addenv(rql, docroot, "SCRIPT_NAME", script) ;
  addrequesturi(rql, docroot, script, infopath) ;
  addenv(rql, docroot, "SERVER_NAME", rql->uri.host) ;
  {
    char proto[9] = "HTTP/1.1" ;
    if (!rql->http_minor) proto[7] = '0' ;
    addenv(rql, docroot, "SERVER_PROTOCOL", proto) ;
  }

  for (size_t i = 0 ; i < hdr->n ; i++)
  {
    char const *key = hdr->buf + hdr->list[i].left ;
    char const *val = hdr->buf + hdr->list[i].right ;
    if (!strcasecmp(key, "Authorization"))
    {
      size_t len = strlen(val) ;
      size_t n = byte_chr(val, len, ' ') ;
      if (n < len)
      {
        addenvb(rql, docroot, "AUTH_TYPE", val, n) ;
        got |= 1 ;
      }
    }
    else if (!strcasecmp(key, "Content-Type")) { addenv(rql, docroot, "CONTENT_TYPE", val) ; got |= 2 ; }
    else if (!strcasecmp(key, "Content-Length") || !strcasecmp(key, "Connection")) ;
    else
    {
      size_t len = strlen(key), pos = g.sa.len + 5 ;
      if (!stralloc_catb(&g.sa, "HTTP_", 5)) die500sys(rql, 111, "stralloc_catb") ;
      addenv(rql, docroot, key, val) ;
      for (char *s = g.sa.s + pos ; len-- ; s++)
        if (*s == '-') *s = '_' ;
        else if (*s >= 'a' && *s <= 'z') *s -= 32 ;
    }
  }
  if (!(got & 1)) delenv(rql, docroot, "AUTH_TYPE") ;
  if (!(got & 2)) delenv(rql, docroot, "CONTENT_TYPE") ;
}

static inline int do_nph (tipidee_rql const *rql, char const *docroot, char const *const *argv, char const *const *envp, char const *body, size_t bodylen) gccattr_noreturn ;
static inline int do_nph (tipidee_rql const *rql, char const *docroot, char const *const *argv, char const *const *envp, char const *body, size_t bodylen)
{
  int p[2] ;
  if (pipe(p) == -1) die500sys(rql, 111, docroot, "pipe") ;
  if (bodylen)
  {
    switch (fork())
    {
      case -1 : die500sys(rql, 111, docroot, "fork") ;
      case 0 :      
      {
#define NAME "tipideed (nph helper for pid "
        tain deadline ;
        size_t m = sizeof(NAME) - 1 ;
        char progstr[sizeof(NAME) + PID_FMT] ;
        memcpy(progstr, NAME, m) ;
        m += pid_fmt(progstr + m, getppid()) ;
        progstr[m++] = ')' ; progstr[m++] = 0 ;
        PROG = progstr ;
        tain_add_g(&deadline, &g.cgitto) ;
        close(p[0]) ;
        if (ndelay_on(p[1]) == -1) die500sys(rql, 111, docroot, "set fd nonblocking") ;
        if (timed_write_g(p[1], body, bodylen, &deadline) < bodylen)
          die500sys(rql, 111, docroot, "write request body to nph ", argv[0]) ;
        _exit(0) ;
      }
      default : break ;
    }
  }
  close(p[1]) ;
  if (fd_move(0, p[0]) == -1) die500sys(rql, 111, docroot, "fd_move") ;
  exec_e(argv, envp) ;
  die500sys(rql, errno == ENOENT ? 127 : 126, docroot, "exec nph ", argv[0]) ;
}

static inline int local_redirect (tipidee_rql *rql, char const *docroot, char const *loc, char *uribuf, char const *cginame)
{
  size_t n ;
  size_t hostlen = strlen(rql->uri.host) ;
  uint16_t port = rql->uri.port ;
  uint8_t ishttps = rql->uri.https ;
  char hosttmp[hostlen + 1] ;
  memcpy(hosttmp, rql->uri.host, hostlen + 1) ;
  n = tipidee_uri_parse(uribuf, URI_BUFSIZE, loc, &rql->uri) ;
  if (!n || n + hostlen + 1 > URI_BUFSIZE)
    die502x(rql, 2, docroot, "cgi ", cginame, " returned an invalid ", "Location", " value", " for local redirection") ;
  memcpy(uribuf + n, hosttmp, hostlen + 1) ;
  rql->uri.host = uribuf + n ;
  rql->uri.port = port ;
  rql->uri.https = ishttps ;
  tipidee_log_debug(g.logv, "cgi ", cginame, ": local redirect to ", rql->uri.path) ;
  return 1 ;
}

static inline int do_cgi (tipidee_rql *rql, char const *docroot, char const *const *argv, char const *const *envp, char const *body, size_t bodylen, char *uribuf, int autochunk)
{
  char const *reason = "OK" ;
  char const *location ;
  char const *contentlength ;
  char const *statusfield ;
  unsigned int status = 0 ;
  size_t rbodylen = 0 ;
  int r ;
  tipidee_headers hdr ;
  char hdrbuf[4096] ;
  char buf[BUFFER_OUTSIZE - 16] ;  /* leave room for chunk encoding */
  iopause_fd x[2] = { { .events = IOPAUSE_READ }, { .events = IOPAUSE_WRITE } } ;
  size_t bodyw = 0 ;
  tain deadline ;
  pid_t pid ;
  disize curheader = DISIZE_ZERO ;
  uint32_t parserstate = 0 ;
  buffer b ;

  tipidee_headers_init(&hdr, hdrbuf, 4096) ;
  {
    int fd[2] = { 0, 1 } ;
    pid = child_spawn2(argv[0], argv, envp, fd) ;
    if (!pid) die500sys(rql, 111, docroot, "spawn ", argv[0]) ;
    x[0].fd = fd[0] ; x[1].fd = fd[1] ;
  }
  if (!bodylen)
  {
    close(x[1].fd) ;
    x[1].fd = -1 ;
  }
  buffer_init(&b, &buffer_read, x[0].fd, buf, BUFFER_OUTSIZE - 16) ;
  tain_add_g(&deadline, &g.cgitto) ;

  for (;;)
  {
    r = iopause_g(x, 1 + (x[1].fd >= 0), &deadline) ;
    if (r == -1) die500sys(rql, 111, docroot, "iopause") ;
    if (!r) goto cgitimeout ;
    if (x[1].fd >= 0 && x[1].revents & IOPAUSE_WRITE)
    {
      size_t len = allwrite(x[1].fd, body + bodyw, bodylen - bodyw) ;
      if (!len)
      {
        strerr_warnwu2sys("write request body to cgi ", argv[0]) ;
        bodyw = bodylen ;
      }
      else bodyw += len ;
      if (bodyw >= bodylen)
      {
        close(x[1].fd) ;
        x[1].fd = -1 ;
      }
    }
    if (x[0].revents & IOPAUSE_READ)
    {
      switch (tipidee_headers_parse_nb(&b, &hdr, &curheader, &parserstate))
      {
        case -2 : break ;
        case -1 : die500sys(rql, 111, docroot, "read from cgi ", argv[0]) ;
        case 0 : goto normalcase ;
        case 400 : die502x(rql, 2, docroot, "invalid output", " from cgi ", argv[0]) ;
        case 413 : die502x(rql, 2, docroot, hdr.n >= TIPIDEE_HEADERS_MAX ? "Too many headers" : "Too much header data", " from cgi ", argv[0]) ;
        case 500 : die500x(rql, 101, docroot, "can't happen: ", "avltreen_insert failed", " in do_cgi") ;
        default : die500x(rql, 101, docroot, "can't happen: ", "unknown tipidee_headers_parse return code", " in do_cgi") ;
      }
    }
  }

 cgitimeout:
  kill(pid, SIGTERM) ;
  strerr_warnw3x("cgi ", argv[0], " timed out") ;
  respond_504(rql, docroot) ;
  if (x[1].fd >= 0) close(x[1].fd) ;
  close(x[0].fd) ;
  return 0 ;

 normalcase:

  if (x[1].fd >= 0)
  {
    if (timed_write_g(x[1].fd, body + bodyw, bodylen - bodyw, &deadline) < bodylen - bodyw)
    {
      if (errno == ETIMEDOUT) goto cgitimeout ;
      else strerr_warnwu2sys("write request body to cgi ", argv[0]) ;
    }
    close(x[1].fd) ;
    x[1].fd = -1 ;
  }
  tain_add_g(&deadline, &g.writetto) ;

  statusfield = tipidee_headers_search(&hdr, "Status") ;
  location = tipidee_headers_search(&hdr, "Location") ;
  contentlength = tipidee_headers_search(&hdr, "Content-Length") ;
  if (contentlength)
  {
    if (!size0_scan(contentlength, &rbodylen))
      die502x(rql, 2, docroot, "cgi ", argv[0], " returned an invalid ", "Content-Length", " header") ;
  }
  if (statusfield)
  {
    size_t m = uint_scan(statusfield, &status) ;
    if (!m || (statusfield[m] && statusfield[m] != ' '))
      die502x(rql, 2, docroot, "cgi ", argv[0], " returned an invalid ", "Status", " header") ;
    if (statusfield[m]) reason = statusfield + m + 1 ;
    else
    {
      tipidee_defaulttext dt ;
      reason = tipidee_util_defaulttext(status, &dt) ? dt.reason : "" ;
    }
    if (!location && (status == 301 || status == 302 || status == 307 || status == 308))
      die502x(rql, 2, docroot, "cgi ", argv[0], " returned a redirection status code without a ", "Location", " header") ;
    if (status < 100 || status > 999)
      die502x(rql, 2, docroot, "cgi ", argv[0], " returned an invalid ", "Status", " value") ;
  }
  if (location)
  {
    if (!location[0]) die502x(rql, 2, docroot, "cgi ", argv[0], " returned an invalid ", "Location", " header") ;
    if (location[0] == '/' && location[1] != '/')
    {
      close(x[0].fd) ;
      return local_redirect(rql, docroot, location, uribuf, argv[0]) ;
    }
    if (rbodylen)
    {
      if (!status)
        die502x(rql, 2, docroot, "cgi ", argv[0], " didn't output a ", "Status", " header", " for a client redirect response with document") ;
      if (status < 300 || status > 399)
        die502x(rql, 2, docroot, "cgi ", argv[0], " returned an invalid ", "Status", " value", " for a client redirect response with document") ;
    }
    else
    {
      for (size_t i = 0 ; i < hdr.n ; i++)
      {
        char const *key = hdr.buf + hdr.list[i].left ;
        if (!strcasecmp(key, "Location") || !strcasecmp(key, "Status")) continue ;
        if (str_start(key, "X-CGI-")) continue ;
        die502x(rql, 2, docroot, "cgi ", argv[0], " returned extra headers", " for a client redirect response without document") ;
      }
      if (!status)
      {
        status = 302 ;
        reason = "Found" ;
      }
    }
  }
  else
  {
    if (!status) status = 200 ;
    if (status != 304 && !tipidee_headers_search(&hdr, "Content-Type"))
      die502x(rql, 2, docroot, "cgi ", argv[0], " didn't output a ", "Content-Type", " header") ;
  }

  tipidee_response_status(buffer_1, rql, status, reason) ;
  tipidee_response_header_writemerge_G(buffer_1, g.rhdr, g.rhdrn, &hdr, !g.cont) ;

  if (contentlength)
  {
    char fmt[SIZE_FMT] ;
    fmt[size_fmt(fmt, rbodylen)] = 0 ;
    buffer_putsnoflush(buffer_1, "Content-Length: ") ;
    buffer_putsnoflush(buffer_1, fmt) ;
    buffer_putnoflush(buffer_1, "\r\n", 2) ;
  }
  else if (autochunk)
    buffer_putsnoflush(buffer_1, "Transfer-Encoding: chunked\r\n") ;

  buffer_putnoflush(buffer_1, "\r\n", 2) ;
  tipidee_log_answer(g.logv, rql, status, rbodylen) ;

  if (contentlength)
  {
    size_t len ;
    struct iovec v[2] ;
    if (rbodylen)
    {
      buffer_wpeek(&b, v) ;
      len = siovec_len(v, 2) ;
      if (len > rbodylen)
      {
        siovec_trunc(v, 2, rbodylen) ;
        len = rbodylen ;
        strerr_warnw3x("cgi ", argv[0], " returned more data than advertised in its Content-Length") ;
      }
      if (buffer_timed_putv_g(buffer_1, v, 2, &deadline) < len)
        strerr_diefu1sys(111, "write to stdout") ;
      rbodylen -= len ;
    }
    if (!rbodylen)
    {
      close(x[0].fd) ;
      return 0 ;
    }
    stream_fixed(x[0].fd, rbodylen, argv[0]) ;
  }
  else if (autochunk)
    stream_autochunk(&b, argv[0]) ;
  else
  {
    size_t len ;
    struct iovec v[2] ;
    buffer_wpeek(&b, v) ;
    len = siovec_len(v, 2) ;
    if (buffer_timed_putv_g(buffer_1, v, 2, &deadline) < len)
      strerr_diefu1sys(111, "write to stdout") ;
    stream_infinite(x[0].fd, argv[0]) ;
  }

  close(x[0].fd) ;
  return 0 ;
}

int respond_cgi (tipidee_rql *rql, char const *docroot, char const *fn, size_t docrootlen, char const *infopath, char *uribuf, tipidee_headers const *hdr, tipidee_resattr const *ra, char const *body, size_t bodylen)
{
  size_t sabase = g.sa.len ;
  size_t envmax = g.envlen + 16 + TIPIDEE_HEADERS_MAX ;
  char const *argv[2] = { fn, 0 } ;
  char const *envp[envmax] ;
  modify_env(rql, docroot, hdr, bodylen, fn + docrootlen, infopath) ;
  env_merge(envp, envmax, (char const *const *)environ, g.envlen, g.sa.s + g.cwdlen + 1, g.sa.len - (g.cwdlen+1)) ;

  g.sa.len = sabase ;
  return ra->flags & TIPIDEE_RA_FLAG_NPH ?
    do_nph(rql, docroot, argv, envp, body, bodylen) :
    do_cgi(rql, docroot, argv, envp, body, bodylen, uribuf, rql->http_minor && ra->flags & TIPIDEE_RA_FLAG_AUTOCHUNK) ;
}
