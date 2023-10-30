/* ISC license. */

#include <skalibs/bsdsnowflake.h>
#include <skalibs/nonposix.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <skalibs/posixplz.h>
#include <skalibs/env.h>
#include <skalibs/uint16.h>
#include <skalibs/types.h>
#include <skalibs/bytestr.h>
#include <skalibs/sgetopt.h>
#include <skalibs/buffer.h>
#include <skalibs/error.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/ip46.h>
#include <skalibs/sig.h>
#include <skalibs/stat.h>
#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/avltreen.h>
#include <skalibs/unix-timed.h>
#include <skalibs/lolstdio.h>

#include <tipidee/tipidee.h>
#include "tipideed-internal.h"

#define USAGE "tipideed [ -f cdbfile ] [ -d basedir ] [ -R ] [ -U ]"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem() strerr_diefu1sys(111, "stralloc_catb")

#define MAX_LOCALREDIRS 32
#define ARGV_MAX 128

struct global_s g = GLOBAL_ZERO ;

static void sigchld_handler (int sig)
{
  (void)sig ;
  wait_reap() ;
}

void log_and_exit (int e)
{
  tipidee_log_exit(g.logv, e) ;
  _exit(e) ;
}

static inline void prep_env (size_t *remoteip, size_t *remotehost)
{
  static char const basevars[] = "PROTO\0TCPCONNNUM\0GATEWAY_INTERFACE=CGI/1.1\0SERVER_SOFTWARE=tipidee/" TIPIDEE_VERSION ;
  static char const sslvars[] = "SSL_PROTOCOL\0SSL_CIPHER\0SSL_TLS_SNI_SERVERNAME\0SSL_PEER_CERT_HASH\0SSL_PEER_CERT_SUBJECT\0HTTPS=on" ;
  char const *x = getenv("SSL_PROTOCOL") ;
  size_t protolen ;
  if (sagetcwd(&g.sa) == -1) strerr_diefu1sys(111, "getcwd") ;
  if (g.sa.len == 1) g.sa.len = 0 ;
  g.cwdlen = g.sa.len ;
  if (!stralloc_readyplus(&g.sa, 220 + sizeof(basevars) + sizeof(sslvars))) dienomem() ;
  if (g.cwdlen) stralloc_0(&g.sa) ;
  stralloc_catb(&g.sa, basevars, sizeof(basevars)) ;
  if (x) stralloc_catb(&g.sa, sslvars, sizeof(sslvars)) ;
  g.ssl = !!x ;
  x = getenv(basevars) ;
  protolen = strlen(x) ;
  if (protolen > 1000) strerr_dieinvalid(100, "PROTO") ;
  if (!x) strerr_dienotset(100, "PROTO")  ;
  {
    size_t m ;
    ip46 ip ;
    uint16_t port ;
    char fmt[IP46_FMT] ;
    char var[protolen + 11] ;
    memcpy(var, x, protolen) ;

    memcpy(var + protolen, "LOCALPORT", 10) ;
    x = getenv(var) ;
    if (!x) strerr_dienotset(100, var) ;
    if (!uint160_scan(x, &g.defaultport)) strerr_dieinvalid(100, var) ;
    if (!stralloc_catb(&g.sa, var, protolen + 10)
     || !stralloc_catb(&g.sa, "SERVER_PORT=", 12)) dienomem() ;
    m = uint16_fmt(fmt, g.defaultport) ; fmt[m++] = 0 ;
    if (!stralloc_catb(&g.sa, fmt, m)) dienomem() ;

    memcpy(var + protolen, "LOCALIP", 8) ;
    x = getenv(var) ;
    if (!x) strerr_dienotset(100, var) ;
    if (!ip46_scan(x, &ip)) strerr_dieinvalid(100, var) ;
    if (!stralloc_catb(&g.sa, var, protolen + 8)
     || !stralloc_catb(&g.sa, "SERVER_ADDR=", 12)) dienomem() ;
    m = ip46_fmt(fmt, &ip) ; fmt[m++] = 0 ;
    if (!stralloc_catb(&g.sa, fmt, m)) dienomem() ;

    memcpy(var + protolen, "LOCALHOST", 10) ;
    x = getenv(var) ;
    if (x)
    {
      if (!stralloc_catb(&g.sa, var, protolen + 10)) dienomem() ;
      g.defaulthost = x ;
    }

    memcpy(var + protolen, "REMOTEPORT", 11) ;
    x = getenv(var) ;
    if (!x) strerr_dienotset(100, var) ;
    if (!uint160_scan(x, &port)) strerr_dieinvalid(100, var) ;
    if (!stralloc_catb(&g.sa, var, protolen + 11)
     || !stralloc_catb(&g.sa, "REMOTE_PORT=", 12)) dienomem() ;
    m = uint16_fmt(fmt, port) ; fmt[m++] = 0 ;
    if (!stralloc_catb(&g.sa, fmt, m)) dienomem() ;

    memcpy(var + protolen, "REMOTEIP", 9) ;
    x = getenv(var) ;
    if (!x) strerr_dienotset(100, var) ;
    if (!ip46_scan(x, &ip)) strerr_dieinvalid(100, var) ;
    if (!stralloc_catb(&g.sa, var, protolen + 9)
     || !stralloc_catb(&g.sa, "REMOTE_ADDR=", 12)) dienomem() ;
    *remoteip = g.sa.len ;
    m = ip46_fmt(fmt, &ip) ; fmt[m++] = 0 ;
    if (!stralloc_catb(&g.sa, fmt, m)) dienomem() ;

    memcpy(var + protolen, "REMOTEHOST", 11) ;
    x = getenv(var) ;
    if ((x && !stralloc_catb(&g.sa, var, protolen + 11))
     || !stralloc_catb(&g.sa, "REMOTE_HOST=", 12)) dienomem() ;
    *remotehost = g.sa.len ;
    if (x)
    {
      if (!stralloc_cats(&g.sa, x)) dienomem() ;
    }
    else
    {
      if (!stralloc_readyplus(&g.sa, m + 2)) dienomem() ;
      if (ip46_is6(&ip)) stralloc_catb(&g.sa, "[", 1) ;
      stralloc_catb(&g.sa, g.sa.s + *remoteip, m) ;
      if (ip46_is6(&ip)) stralloc_catb(&g.sa, "]", 1) ;
    }
    if (!stralloc_0(&g.sa)) dienomem() ;

    memcpy(var + protolen, "REMOTEINFO", 11) ;
    x = getenv(var) ;
    if (x)
      if (!stralloc_catb(&g.sa, var, protolen + 11)
       || !stralloc_catb(&g.sa, "REMOTE_IDENT=", 13)
       || !stralloc_cats(&g.sa, x) || !stralloc_0(&g.sa)) dienomem() ;
  }
}

static uint32_t get_uint32 (char const *key)
{
  uint32_t n ;
  if (!tipidee_conf_get_uint32(&g.conf, key, &n))
    strerr_diefu2sys(102, "read config value for ", key) ;
  return n ;
}

static void inittto (tain *tto, char const *key)
{
  uint32_t ms = get_uint32(key) ;
  if (ms) tain_from_millisecs(tto, ms) ;
  else *tto = tain_infinite_relative ;
}

static inline unsigned int indexify (tipidee_rql const *rql, char const *docroot, char *s, struct stat *st)
{
  unsigned int e = 0 ;
  size_t len = strlen(s) ;
  unsigned int i = 0 ;
  if (s[len - 1] != '/')
  {
    s[len++] = '/' ;
    e = 308 ;
  }
  for (; i < g.indexn ; i++)
  {
    strcpy(s + len, g.indexnames[i]) ;
    if (stat(s, st) == 0) break ;
    switch (errno)
    {
      case EACCES : return 403 ;
      case ENAMETOOLONG : return 414 ;
      case ENOTDIR : return 404 ;
      case ENOENT : continue ;
      default : die500sys(rql, 111, docroot, "stat ", s) ;
    }
  }
  if (i >= g.indexn) return 404 ;
  if (S_ISDIR(st->st_mode)) die500x(rql, 102, docroot, "bad document hierarchy: ", s, " is a directory") ;
  if (e == 308) s[len] = 0 ;
  return e ;
}

static inline void get_resattr (tipidee_rql const *rql, char const *docroot, char const *res, tipidee_resattr *ra)
{
  size_t pos = g.sa.len ;
  if (sarealpath(&g.sa, res) == -1 || !stralloc_0(&g.sa)) die500sys(rql, 111, docroot, "realpath ", res) ;
  if (strncmp(g.sa.s + pos, g.sa.s, g.cwdlen) || g.sa.s[pos + g.cwdlen] != '/')
    die500x(rql, 102, docroot, "resource ", res, " points outside of the server's root") ;
  if (!tipidee_conf_get_resattr(&g.conf, g.sa.s + pos + g.cwdlen + 1, ra))
    die500sys(rql, 102, docroot, "look up resource attributes for ", g.sa.s + pos + g.cwdlen + 1) ;
  g.sa.len = pos ;
}

static inline void force_redirect (tipidee_rql const *rql, char const *fn)
{
  tipidee_redirection rd = { .sub = 0, .type = 1 } ;
  size_t len = strlen(fn) ;
  char location[len + 8 + g.ssl] ;
  memcpy(location, g.ssl ? "https://" : "http://", 7 + g.ssl) ;
  memcpy(location + 7 + g.ssl, fn, len + 1) ;
  rd.location = location ;
  respond_30x(rql, &rd) ;
}

static inline int serve (tipidee_rql *rql, char const *docroot, char *uribuf, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  tipidee_resattr ra = TIPIDEE_RESATTR_ZERO ;
  size_t docrootlen = strlen(docroot) ;
  size_t pathlen = strlen(rql->uri.path) ;
  char const *infopath = 0 ;
  struct stat st ;
  char fn[docrootlen + pathlen + 2 + g.indexlen] ;
  memcpy(fn, docroot, docrootlen) ;
  memcpy(fn + docrootlen, rql->uri.path, pathlen) ;
  fn[docrootlen + pathlen] = 0 ;

 /* Redirection */

  if (rql->m != TIPIDEE_METHOD_OPTIONS)
  {
    tipidee_redirection rd = TIPIDEE_REDIRECTION_ZERO ;
    int e = tipidee_conf_get_redirection(&g.conf, docroot, docrootlen, rql->uri.path, &rd) ;
    if (e == -1) die500sys(rql, 111, docroot, "get redirection data for ", fn) ;
    if (e)
    {
      respond_30x(rql, &rd) ;
      return 0 ;
    }
  }

 /* Resource in the filesystem */

  if (stat(fn, &st) == -1)
  {
    size_t pos = docrootlen + pathlen - 1 ;
    for (;;)
    {
      while (fn[pos] != '/') pos-- ;
      if (pos <= docrootlen) { respond_404(rql, docroot) ; return 0 ; }
      fn[pos] = 0 ;
      if (stat(fn, &st) == 0) break ;
      switch (errno)
      {
        case ENOTDIR :
        case ENOENT : fn[pos--] = '/' ; break ;
        case EACCES : respond_403(rql, docroot) ; return 0 ;
        case ENAMETOOLONG : respond_414(rql, docroot) ; return 0 ;
        default : die500sys(rql, 111, docroot, "stat ", fn) ;
      }
    }
    infopath = fn + pos + 1 ;
  }
  if (S_ISDIR(st.st_mode))
  {
    if (infopath) { respond_404(rql, docroot) ; return 0 ; }
    switch (indexify(rql, docroot, fn, &st))
    {
      case 403 : respond_403(rql, docroot) ; return 0 ;
      case 404 : respond_404(rql, docroot) ; return 0 ;
      case 414 : respond_414(rql, docroot) ; return 0 ;
      case 308 : force_redirect(rql, fn) ; return 0 ;
      case 0 : break ;
    }
  }
  tipidee_log_debug(g.logv, "serve: docroot ", docroot, " file ", fn, " infopath ", infopath ? infopath : "(none)") ;

  get_resattr(rql, docroot, fn, &ra) ;

  if (!ra.flags & TIPIDEE_RA_FLAG_CGI)
  {
    if (infopath) { respond_404(rql, docroot) ; return 0 ; }
    if (rql->m == TIPIDEE_METHOD_POST) exit_405(rql) ;
  }

  if (rql->m == TIPIDEE_METHOD_OPTIONS)
    return respond_options(rql, 2 | !!(ra.flags & TIPIDEE_RA_FLAG_CGI)) ;

  tipidee_log_resource(g.logv, rql, fn, &ra, infopath) ;

  if (ra.flags & TIPIDEE_RA_FLAG_CGI)
    return respond_cgi(rql, docroot, fn, docrootlen, infopath, uribuf, hdr, &ra, body, bodylen) ;

  infopath = tipidee_headers_search(hdr, "If-Modified-Since") ;
  if (infopath)
  {
    tain wanted, actual ;
    if (tipidee_util_httpdate(infopath, &wanted)
     && tain_from_timespec(&actual, &st.st_mtim)
     && tain_less(&actual, &wanted))
      return respond_304(rql, fn, &st) ;
  }
  return respond_regular(rql, docroot, fn, &st, &ra) ;
}

int main (int argc, char const *const *argv, char const *const *envp)
{
  stralloc bodysa = STRALLOC_ZERO ;
  size_t remoteip, remotehost ;
  char const *x ;
  uint32_t n ;
  char progstr[14 + PID_FMT] = "tipideed: pid " ;
  progstr[14 + pid_fmt(progstr + 14, getpid())] = 0 ;
  PROG = progstr ;

  {
    char const *conffile = TIPIDEE_SYSCONFPREFIX "tipidee.conf.cdb" ;
    char const *newroot = 0 ;
    unsigned int h = 0 ;
    subgetopt l = SUBGETOPT_ZERO ;

    for (;;)
    {
      int opt = subgetopt_r(argc, argv, "f:d:RU", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'f' : conffile = l.arg ; break ;
        case 'd' : newroot = l.arg ; break ;
        case 'R' : h |= 3 ; break ;
        case 'U' : h |= 1 ; break ;
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;

    g.envlen = env_len(envp) ;
    if (!tipidee_conf_init(&g.conf, conffile))
      strerr_diefu2sys(111, "find configuration in ", conffile) ;
    if (newroot && chdir(newroot) == -1)
      strerr_diefu2sys(111, "chdir to ", newroot) ;
    tipideed_harden(h) ;
  }

  prep_env(&remoteip, &remotehost) ;
  inittto(&g.readtto, "G:read_timeout") ;
  inittto(&g.writetto, "G:write_timeout") ;
  inittto(&g.cgitto, "G:cgi_timeout") ;
  g.maxrqbody = get_uint32("G:max_request_body_length") ;
  g.maxcgibody = get_uint32("G:max_cgi_body_length") ;
  g.logv = get_uint32("G:logv") ;
  n = tipidee_conf_get_argv(&g.conf, "G:index-file", g.indexnames, 16, &g.indexlen) ;
  if (!n) strerr_dief3x(102, "bad", " config value for ", "G:index_file") ;
  g.indexn = n-1 ;

  x = tipidee_conf_get_responseheaders(&g.conf, "G:response_headers", &n, &g.rhdrn) ;
  if (!x) strerr_diefu3sys(102, "get", " config value for ", "G:response_headers") ;

  tipidee_response_header rhdr[n ? n : 1] ;  /* should start a block but that's a lot of editing */
  if (!tipidee_response_header_preparebuiltin(rhdr, g.rhdrn, x, n))
    strerr_dief3x(102, "bad", " config value for ", "G:response_headers") ;
  g.rhdr = rhdr ;

  if (ndelay_on(0) == -1 || ndelay_on(1) == -1)
    strerr_diefu1sys(111, "set I/O nonblocking") ;
  init_splice_pipe() ;
  if (!sig_catch(SIGCHLD, &sigchld_handler))
    strerr_diefu1sys(111, "set SIGCHLD handler") ;
  if (!sig_altignore(SIGPIPE))
    strerr_diefu1sys(111, "ignore SIGPIPE") ;
  if (!tain_now_set_stopwatch_g())
    strerr_diefu1sys(111, "initialize clock") ;


  tipidee_log_start(g.logv, g.sa.s + remoteip, g.sa.s + remotehost) ;


 /* Main loop */

  while (g.cont)
  {
    tain deadline ;
    tipidee_rql rql = TIPIDEE_RQL_ZERO ;
    tipidee_headers hdr ;
    int e ;
    unsigned int localredirs = 0 ;
    size_t content_length ;
    tipidee_transfercoding tcoding = TIPIDEE_TRANSFERCODING_UNKNOWN ;
    char uribuf[URI_BUFSIZE] ;
    char hdrbuf[HDR_BUFSIZE] ;

    tain_add_g(&deadline, &g.readtto) ;
    bodysa.len = 0 ;

    e = tipidee_rql_read_g(buffer_0, uribuf, URI_BUFSIZE, &content_length, &rql, &deadline) ;
    switch (e)
    {
      case -1 : log_and_exit(1) ;  /* bad client */
      case 0 : break ;
      case 98 :  /* client exited */
      case 99 : g.cont = 0 ; continue ;  /* timeout */
      case 400 : eexit_400(&rql, "Syntax error in request line") ;
      default : strerr_dief2x(101, "can't happen: ", "unknown tipidee_rql_read return code") ;
    }
    if (rql.http_major != 1) log_and_exit(1) ;
    if (rql.http_minor > 1) eexit_400(&rql, "Bad HTTP version") ;

    content_length = 0 ;
    tipidee_headers_init(&hdr, hdrbuf, HDR_BUFSIZE) ;
    e = tipidee_headers_timed_parse_g(buffer_0, &hdr, &deadline) ;
    switch (e)
    {
      case -1 : log_and_exit(1) ;  /* connection issue, client timeout, etc. */
      case 0 : break ;
      case 400 : eexit_400(&rql, "Syntax error in headers") ;
      case 408 : eexit_408(&rql) ;  /* timeout */
      case 413 : eexit_413(&rql, hdr.n >= TIPIDEE_HEADERS_MAX ? "Too many headers" : "Too much header data") ;
      case 500 : strerr_dief2x(101, "can't happen: ", "avltreen_insert failed") ;
      default : strerr_dief2x(101, "can't happen: ", "unknown tipidee_headers_parse return code") ;
    }

    if (!rql.http_minor) g.cont = 0 ;
    else
    {
      x = tipidee_headers_search(&hdr, "Connection") ;
      if (x)
      {
        if (strcasestr(x, "close")) g.cont = 0 ;
        else if (strcasestr(x, "keep-alive")) g.cont = 2 ;
      }
    }

    x = tipidee_headers_search(&hdr, "Transfer-Encoding") ;
    if (x)
    {
      if (strcasecmp(x, "chunked")) eexit_400(&rql, "unsupported Transfer-Encoding") ;
      else tcoding = TIPIDEE_TRANSFERCODING_CHUNKED ;
    }
    else
    {
      x = tipidee_headers_search(&hdr, "Content-Length") ;
      if (x)
      {
        if (!size_scan(x, &content_length)) eexit_400(&rql, "Invalid Content-Length") ;
        else if (content_length) tcoding = TIPIDEE_TRANSFERCODING_FIXED ;
        else tcoding = TIPIDEE_TRANSFERCODING_NONE ;
      }
      else tcoding = TIPIDEE_TRANSFERCODING_NONE ;
    }

    if (tcoding != TIPIDEE_TRANSFERCODING_NONE && rql.m != TIPIDEE_METHOD_POST)
      eexit_400(&rql, "only POST requests can have an entity body") ;

    switch (rql.m)
    {
      case TIPIDEE_METHOD_GET :
      case TIPIDEE_METHOD_HEAD :
      case TIPIDEE_METHOD_POST :
      case TIPIDEE_METHOD_TRACE : break ;
      case TIPIDEE_METHOD_OPTIONS :
        if (!rql.uri.path) { respond_options(&rql, 1) ; continue ; }
        break ;
      case TIPIDEE_METHOD_PUT :
      case TIPIDEE_METHOD_DELETE : eexit_405(&rql) ;
      case TIPIDEE_METHOD_CONNECT : eexit_501(&rql, "CONNECT method unsupported") ;
      case TIPIDEE_METHOD_PRI : eexit_501(&rql, "PRI method attempted with HTTP version 1") ;
      default : strerr_dief2x(101, "can't happen: ", "unknown HTTP method") ;
    }

    x = tipidee_headers_search(&hdr, "Host") ;  /* HTTP/1.05 */
    if (x)
    {
      char *p = strchr(x, ':') ;
      if (p)
      {
        if (!uint160_scan(p+1, &rql.uri.port)) eexit_400(&rql, "Invalid Host header") ;
        *p = 0 ;
      }
      if (!*x || *x == '.') eexit_400(&rql, "Invalid Host header") ;
      rql.uri.host = x ;
    }
    else if (!rql.uri.host)
    {
      if (rql.http_minor) eexit_400(&rql, "Missing Host header") ;
      else rql.uri.host = g.defaulthost ;
    }
    if (!rql.uri.port) rql.uri.port = g.defaultport ;
    tipidee_log_request(g.logv, &rql, &hdr, &g.sa) ;

    if (rql.m == TIPIDEE_METHOD_TRACE)
    {
      respond_trace(hdrbuf, &rql, &hdr) ;
      continue ;
    }

    {
      size_t hostlen = strlen(rql.uri.host) ;
      char docroot[hostlen + UINT16_FMT + 1] ;
      if (rql.uri.host[hostlen - 1] == '.') hostlen-- ;
      memcpy(docroot, rql.uri.host, hostlen) ;
      docroot[hostlen] = ':' ;
      docroot[hostlen + 1 + uint16_fmt(docroot + hostlen + 1, rql.uri.port)] = 0 ;

     /* All good. Read the body if any */

      switch (tcoding)
      {
        case TIPIDEE_TRANSFERCODING_FIXED :
        {
          if (content_length > g.maxrqbody) exit_413(&rql, docroot) ;
          if (!stralloc_ready(&bodysa, content_length)) die500sys(&rql, 111, docroot, "stralloc_ready") ;
          if (buffer_timed_get_g(buffer_0, bodysa.s, content_length, &deadline) < content_length)
          {
            if (errno == ETIMEDOUT) exit_408(&rql, docroot) ;
            else exit_400(&rql, docroot) ;
          }
          bodysa.len = content_length ;
          break ;
        }
        case TIPIDEE_TRANSFERCODING_CHUNKED :
        {
          if (!tipidee_util_chunked_read_g(buffer_0, &bodysa, g.maxrqbody, &deadline))
          {
            if (error_temp(errno)) die500sys(&rql, 111, docroot, "decode chunked body") ;
            else if (errno == EMSGSIZE) exit_413(&rql, docroot) ;
            else exit_400(&rql, docroot) ;
          }
          break ;
        }
        default : break ;
      }


     /* Find and serve the resource. The loop is in case of CGI local-redirection. */

      while (serve(&rql, docroot, uribuf, &hdr, bodysa.s, bodysa.len))
        if (localredirs++ >= MAX_LOCALREDIRS)
          die502x(&rql, 2, docroot, "too many local redirections - possible loop involving path ", rql.uri.path) ;
    }
  }

  log_and_exit(0) ;
}
