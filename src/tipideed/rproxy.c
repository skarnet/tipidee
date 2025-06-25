/* ISC license. */

#include <stdint.h>
#include <unistd.h>
#include <strings.h>

#include <skalibs/uint16.h>
#include <skalibs/fmtscan.h>
#include <skalibs/tai.h>
#include <skalibs/buffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/socket.h>
#include <skalibs/ip46.h>
#include <skalibs/unix-timed.h>

#include <tipidee/tipidee.h>
#include "tipideed-internal.h"

#define putit(b, s, deadline) if (buffer_timed_puts_g(b, s, deadline) == -1) die500sys(rql, 111, docroot, "write to ", fn)

static void do_rproxy (tipidee_rql const *rql, int fd, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen, tain const *deadline, char const *fn)
{
  char buf[4096] ;
  buffer b = BUFFER_INIT(&buffer_write, fd, buf, 4096) ;
  putit(&b, tipidee_method_tostr(rql->m), deadline) ;
  putit(&b, " ", deadline) ;
  putit(&b, rql->uri.path, deadline) ;
  if (rql->uri.query)
  {
    putit(&b, "?", deadline) ;
    putit(&b, rql->uri.query, deadline) ;
  }
  putit(&b, " HTTP/1.", deadline) ;
  putit(&b, rql->http_minor == 1 ? "1" : "0", deadline) ;
  putit(&b, "\r\nConnection: close\r\n", deadline) ;
  for (uint32_t i = 0 ; i < hdr->n ; i++)
  {
    if (!strcasecmp(hdr->buf + hdr->list[i].left, "Connection")) continue ;
    putit(&b, hdr->buf + hdr->list[i].left, deadline) ;
    putit(&b, ": ", deadline) ;
    putit(&b, hdr->buf + hdr->list[i].right, deadline) ;
    putit(&b, "\r\n", deadline) ;
  }
  putit(&b, "\r\n", deadline) ;
  if (!buffer_timed_flush_g(&b, deadline)) die500sys(rql, 111, docroot, "write to ", fn) ;
  if (bodylen)
  {
    if (timed_write_g(fd, body, bodylen, deadline) < bodylen) die500sys(rql, 111, docroot, "write to ", fn) ;
  }
  fd_shutdown(fd, 1) ;
  if (ndelay_off(fd) == -1) die500sys(rql, 111, docroot, "set socket parameters for ", fn) ;
  fd_cat(fd, 1) ;
  _exit(0) ;  /* expensive but the only way to avoid implementing a full proxy */
}

static void rproxy_unix (tipidee_rql const *rql, char const *socketpath, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  tain deadline ;
  int fd = ipc_stream_nbcoe() ;
  if (fd == -1) die500sys(rql, 111, docroot, "create socket") ;
  tain_add_g(&deadline, &g.readtto) ;
  if (!ipc_timed_connect_g(fd, socketpath, &deadline)) die500sys(rql, 111, docroot, "connect to ", socketpath) ;
  do_rproxy(rql, fd, sub, docroot, hdr, body, bodylen, &deadline, socketpath) ;
  fd_close(fd) ;
}

static void rproxy_tcp (tipidee_rql const *rql, char const *ip, uint16_t port, int is6, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  tain deadline ;
  size_t m = 0 ;
  int fd = socket_tcp46_nbcoe(is6) ;
  char fmt[IP6_FMT + UINT16_FMT + 2] ;
  if (fd == -1) die500sys(rql, 111, docroot, "create socket") ;
  tain_add_g(&deadline, &g.readtto) ;
  if (is6) fmt[m++] = '[' ;
  m += is6 ? ip6_fmt(fmt + m, ip) : ip4_fmt(fmt + m, ip) ;
  if (is6) fmt[m++] = ']' ;
  fmt[m++] = ':' ;
  m += uint16_fmt(fmt + m, port) ;
  fmt[m++] = 0 ;
  
  if (!(is6 ? socket_deadlineconnstamp6_g(fd, ip, port, &deadline) : socket_deadlineconnstamp4_g(fd, ip, port, &deadline)))
    die500sys(rql, 111, docroot, "connect to ", fmt) ;
  do_rproxy(rql, fd, sub, docroot, hdr, body, bodylen, &deadline, fmt) ;
  fd_close(fd) ;
}

void rproxy (tipidee_rql const *rql, tipidee_redirection const *rd, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  if (rd->type & 16) rproxy_unix(rql, rd->location, rd->sub, docroot, hdr, body, bodylen) ;
  else
  {
    uint16_t port ;
    uint16_unpack_big(rd->location, &port) ;
    rproxy_tcp(rql, rd->location + 2, port, !!(rd->type & 8), rd->sub, docroot, hdr, body, bodylen) ;
  }
}
