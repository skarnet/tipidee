/* ISC license. */

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include <skalibs/gccattributes.h>
#include <skalibs/uint16.h>
#include <skalibs/bytestr.h>
#include <skalibs/disize.h>
#include <skalibs/fmtscan.h>
#include <skalibs/tai.h>
#include <skalibs/buffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>
#include <skalibs/ip46.h>
#include <skalibs/unix-timed.h>

#include <tipidee/tipidee.h>
#include "tipideed-internal.h"

typedef struct fctx_s fctx, *fctx_ref ;
struct fctx_s
{
  tipidee_fcgi fc ;
  tipidee_rql *rql ;
  char const *sub ;
  char const *docroot ;
  char const *fn ;
} ;


/*
  Scan g.sa for environment variables


st\ev	\0	=	other
	0	1	2

0			m
KSTART	X	X	KEY

1		k
KEY	KSTART	VSTART	KEY

2	mv	m	m
VSTART	KSTART	VALUE	VALUE

3	v
VALUE	KSTART	VALUE	VALUE

4
X

0x10	k	store key start and length
0x20	m	mark
0x40	v	store value start and length

*/

static uint8_t cclass (char c) gccattr_pure ;
static uint8_t cclass (char c)
{
  switch (c)
  {
    case 0 : return 0 ;
    case '=' : return 1 ;
    default : return 2 ;
  }
}

static int parse_vars (stralloc *sa, char const *s, size_t len)
{
  static uint8_t table[4][3] =
  {
    { 0x04, 0x04, 0x21 },
    { 0x00, 0x12, 0x01 },
    { 0x60, 0x23, 0x23 },
    { 0x40, 0x03, 0x03 }
  } ;
  size_t keystart = 0, keylen = 0, mark = 0 ;
  uint8_t state = 0 ;
  for (size_t i = 0 ; i < len ; i++)
  {
    uint8_t c = table[state][cclass(s[i])] ;
    state = c & 0x07 ;
    if (state >= 4) return (errno = EDOM, 0) ;
    if (c & 0x10) { keystart = mark ; keylen = i - mark ; }
    if (c & 0x20) mark = i ;
    if (c & 0x40)
    {
      if (!tipidee_fcgi_addenvb(sa, s + keystart, keylen, s + mark, i - mark)) return 0 ;
    }
  }
  return 1 ;
}

static void addenv (fctx *c, stralloc *sa, char const *key, char const *val)
{
  if (!tipidee_fcgi_addenv(sa, key, val))
  {
    if (errno != E2BIG) die500sys(c->rql, 111, c->docroot, "prepare environment for ", "fastcgi", " ", c->fn) ;
    if (!tipidee_fcgi_params_g(&c->fc, sa->s, sa->len)) die500sys(c->rql, 111, c->docroot, "send environment to ", "fastcgi", " ", c->fn) ;
    sa->len = 0 ;
    if (!tipidee_fcgi_addenv(sa, key, val)) die500sys(c->rql, 111, c->docroot, "prepare environment for ", "fastcgi", " ", c->fn) ;
  }
}

static void prepare_env (fctx *c, tipidee_headers const *hdr, size_t bodylen, stralloc *sa)
{
  size_t docrootlen = strlen(c->docroot) ;
  sa->len = 0 ;
  if (!parse_vars(sa, g.sa.s + g.cwdlen + 1, g.sa.len - (g.cwdlen + 1)))
  {
    if (errno == EDOM) die500sys(c->rql, 102, c->docroot, "can't happen: invalid environment string") ;
    else die500sys(c->rql, 111, c->docroot, "encode ", "environment string for ", "fastcgi", " ", c->fn) ;
  }
  if (bodylen)
  {
    char fmt[SIZE_FMT] ;
    fmt[size_fmt(fmt, bodylen)] = 0 ;
    addenv(c, sa, "CONTENT_LENGTH", fmt) ;
  }
  if (c->rql->uri.query)
  {
    size_t pathlen = strlen(c->rql->uri.path) ;
    size_t querylen = strlen(c->rql->uri.query) ;
    char val[pathlen + querylen + 2] ;
    memcpy(val, c->rql->uri.path, pathlen) ;
    val[pathlen] = '?' ;
    memcpy(val + pathlen + 1, c->rql->uri.query, querylen + 1) ;
    addenv(c, sa, "REQUEST_URI", val) ;
  }
  else addenv(c, sa, "REQUEST_URI", c->rql->uri.path) ;
  addenv(c, sa, "REQUEST_METHOD", tipidee_method_tostr(c->rql->m)) ;
  addenv(c, sa, "QUERY_STRING", c->rql->uri.query ? c->rql->uri.query : "") ;
  if (c->sub)
  {
    size_t sublen = strlen(c->sub) ;
    size_t pathlen = strlen(c->rql->uri.path) ;
    char val[pathlen - sublen - docrootlen + 1] ;
    memcpy(val, c->rql->uri.path + docrootlen, pathlen - sublen - docrootlen) ;
    val[pathlen - sublen - docrootlen] = 0 ;
    addenv(c, sa, "PATH_INFO", c->sub) ;
    addenv(c, sa, "SCRIPT_NAME", val) ;
  }
  else addenv(c, sa, "SCRIPT_NAME", c->rql->uri.path + docrootlen) ;
  addenv(c, sa, "SERVER_NAME", c->rql->uri.host) ;
  addenv(c, sa, "SERVER_PROTOCOL", c->rql->http_minor ? "HTTP/1.1" : "HTTP/1.0") ;

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
        char tmp[n+1] ;
        memcpy(tmp, val, n) ;
        tmp[n] = 0 ;
        addenv(c, sa, "AUTH_TYPE", tmp) ;
      }
      if (g.flagcgipassauth) addenv(c, sa, "HTTP_AUTHORIZATION", val) ;
    }
    else if (!strcasecmp(key, "Content-Type")) addenv(c, sa, "CONTENT_TYPE", val) ;
    else if (!strcasecmp(key, "Content-Length") || !strcasecmp(key, "Connection")) ;
    else
    {
      size_t keylen = strlen(key) ;
      char tmp[keylen + 6] ;
      memcpy(tmp, "HTTP_", 5) ;
      for (size_t i = 0 ; i < keylen ; i++)
        tmp[5 + i] = key[i] == '-' ? '_' : key[i] >= 'a' && key[i] <= 'z' ? key[i] - 32 : key[i] ;
      tmp[5 + keylen] = 0 ;
      addenv(c, sa, tmp, val) ;
    }
  }
  if (sa->len && !tipidee_fcgi_params_g(&c->fc, sa->s, sa->len))
    die500sys(c->rql, 111, c->docroot, "send", " environment to ", "fastcgi", " ", c->fn) ;
  sa->len = 0 ;
  if (!tipidee_fcgi_params_g(&c->fc, 0, 0))
    die500sys(c->rql, 111, c->docroot, "finish sending", " environment to ", "fastcgi", " ", c->fn) ;
}

static void do_write (fctx *c, char const *s, size_t len, uint64_t cl, uint64_t *w)
{
  tain wdeadline ;
  tain_add_g(&wdeadline, &g.writetto) ;
  if (cl + len > *w)
    die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " sent more data than advertised") ;
  if (!cl)
  {
    char fmt[SIZE_XFMT] ;
    size_t w = size_xfmt(fmt, len) ;
    if (buffer_timed_put_g(buffer_1, fmt, w, &wdeadline) < w
     || buffer_timed_put_g(buffer_1, "\r\n", 2, &wdeadline) < 2)
      strerr_diefusys(111, "write to stdout") ;
  }
  if (buffer_timed_put_g(buffer_1, s, len, &wdeadline) < len)
    strerr_diefusys(111, "write to stdout") ;
  if (!cl && buffer_timed_put_g(buffer_1, "\r\n", 2, &wdeadline) < 2)
    strerr_diefusys(111, "write to stdout") ;
  if (!buffer_timed_flush_g(buffer_1, &wdeadline))
    strerr_diefusys(111, "write to stdout") ;
  *w += len ;
}

static int do_fastcgi (fctx *c, int fd, tipidee_headers const *hdr, char const *body, size_t bodylen, tain const *deadline, char *uribuf)
{
  static stralloc sa = STRALLOC_ZERO ;
  tipidee_headers rhdr ;
  char rhdrbuf[4096] ;
  uint64_t cl = 0 ;
  uint64_t w = 0 ;
  uint32_t status = 0 ;
  disize curheader = DISIZE_ZERO ;
  uint32_t parserstate = 0 ;
  int stream = 1 ;
  int state = 0 ;

  tipidee_headers_init(&rhdr, rhdrbuf, 4096) ;
  tipidee_fcgi_startwrite(&c->fc, fd, getpid(), deadline) ;
  if (!tipidee_fcgi_beginrequest_g(&c->fc)) die500sys(c->rql, 111, c->docroot, "send request to ", "fastcgi", " ", c->fn) ;
  prepare_env(c, hdr, bodylen, &sa) ;
  if (bodylen && !tipidee_fcgi_stdin_g(&c->fc, body, bodylen)) die500sys(c->rql, 111, c->docroot, "send", " request body to ", "fastcgi", " ", c->fn) ;
  if (!tipidee_fcgi_stdin_g(&c->fc, 0, 0)) die500sys(c->rql, 111, c->docroot, "finish sending", " request body to ", "fastcgi", " ", c->fn) ;

  tipidee_fcgi_startread(&c->fc, fd, c->fc.id, deadline) ;
  while (stream)
  {
    stream = tipidee_fcgi_read_g(&c->fc, &status, &sa) ;
    if (stream == -1)
    {
      switch (errno)
      {
        case ECONNREFUSED: die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " said it can only process one connection") ;
        case ENFILE: die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " is overloaded") ;
        case EAFNOSUPPORT: die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " does not support the Responder role") ;
        case EPROTO : die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " spoke a weird dialect") ;
        default : die502sys(c->rql, 111, c->docroot, "unable to ", "read", " from ", "fastcgi", " ", c->fn) ;
      }
    }
    else if (stream == 1)
    {
      if (!state)
      {
        size_t i = 0 ;
        int r ;
        switch (tipidee_headers_parse_fromstring_nb(sa.s, sa.len, &i, &rhdr, &curheader, &parserstate))
        {
          case -2 : goto cont ;
          case -1 : die500sys(c->rql, 111, c->docroot, "read", " from ", "fastcgi", " ", c->fn) ;
          case 0 : break ;
          case 400 : die502x(c->rql, 2, c->docroot, "invalid output", " from ", "fastcgi", " ", c->fn) ;
          case 413 : die502x(c->rql, 2, c->docroot, rhdr.n >= TIPIDEE_HEADERS_MAX ? "Too many headers" : "Too much header data", " from ", "fastcgi", " ", c->fn) ;
          case 500 : die500x(c->rql, 101, c->docroot, "can't happen: ", "avltreen_insert failed", " in do_fastcgi") ;
          default : die500x(c->rql, 101, c->docroot, "can't happen: ", "unknown tipidee_headers_parse return code", " in do_fastcgi") ;
        }

        r = process_cgi_headers(c->rql, &rhdr, c->docroot, "fastcgi", c->fn, 1, &cl) ;
        if (r == 2)
        {
          fd_close(fd) ;
          return local_redirect(c->rql, c->docroot, tipidee_headers_search(&rhdr, "Location"), uribuf, "fastcgi", c->fn) ;
        }
        if (c->rql->m == TIPIDEE_METHOD_HEAD)
        {
          tain wdeadline ;
          tain_add_g(&wdeadline, &g.writetto) ;
          if (!buffer_timed_flush_g(buffer_1, &wdeadline))
            strerr_diefu1sys(111, "write to stdout") ;
          return 0 ;
        }
        state = 1 ;
        if (i < sa.len) do_write(c, sa.s + i, sa.len - i, cl, &w) ;
      }
      else if (state == 1) do_write(c, sa.s, sa.len, cl, &w) ;
      else die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " kept sending FCGI_STDOUT data after EOF marker") ;
     cont:
      if (!sa.len) state = 2 ;
      sa.len = 0 ;
    }
    else if (stream == 2)
    {
      buffer_putflush(buffer_2, sa.s, sa.len) ;
      tain_now_g() ;
      sa.len = 0 ;
    }
    else die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " sent data on an unsupported stream") ;
  }
  fd_close(fd) ;
  if (cl && w < cl)
    die502x(c->rql, 111, c->docroot, "fastcgi", " ", c->fn, " sent less data than advertised") ;
  return 0 ;
}

static int fastcgi_unix (fctx *c, char const *socketpath, tipidee_headers const *hdr, char const *body, size_t bodylen, char *uribuf)
{
  tain deadline ;
  int fd = ipc_stream_nbcoe() ;
  if (fd == -1) die500sys(c->rql, 111, c->docroot, "create socket") ;
  tain_add_g(&deadline, &g.readtto) ;
  if (!ipc_timed_connect_g(fd, socketpath, &deadline)) die500sys(c->rql, 111, c->docroot, "connect to ", socketpath) ;
  c->fn = socketpath ;
  return do_fastcgi(c, fd, hdr, body, bodylen, &deadline, uribuf) ;
}

static inline int fastcgi_tcp (fctx *c, char const *ip, uint16_t port, int is6, tipidee_headers const *hdr, char const *body, size_t bodylen, char *uribuf)
{
  tain deadline ;
  size_t m = 0 ;
  int fd = socket_tcp46_nbcoe(is6) ;
  char fmt[IP6_FMT + UINT16_FMT + 2] ;
  if (fd == -1) die500sys(c->rql, 111, c->docroot, "create socket") ;
  tain_add_g(&deadline, &g.cgitto) ;
  if (is6) fmt[m++] = '[' ;
  m += is6 ? ip6_fmt(fmt + m, ip) : ip4_fmt(fmt + m, ip) ;
  if (is6) fmt[m++] = ']' ;
  fmt[m++] = ':' ;
  m += uint16_fmt(fmt + m, port) ;
  fmt[m++] = 0 ;
  if (!(is6 ? socket_deadlineconnstamp6_g(fd, ip, port, &deadline) : socket_deadlineconnstamp4_g(fd, ip, port, &deadline)))
    die500sys(c->rql, 111, c->docroot, "connect to ", fmt) ;
  c->fn = fmt ;
  return do_fastcgi(c, fd, hdr, body, bodylen, &deadline, uribuf) ;
}

int fastcgi (tipidee_rql *rql, tipidee_redirection const *rd, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen, char *uribuf)
{
  fctx c = { .fc = TIPIDEE_FCGI_ZERO, .rql = rql, .sub = rd->sub, .docroot = docroot, .fn = 0 } ;
  if (rd->flags & TIPIDEE_REDIR_ISINET)
    return fastcgi_tcp(&c, rd->addr, rd->port, !!(rd->flags & TIPIDEE_REDIR_ISV6), hdr, body, bodylen, uribuf) ;
  else
    return fastcgi_unix(&c, rd->addr, hdr, body, bodylen, uribuf) ;
}
