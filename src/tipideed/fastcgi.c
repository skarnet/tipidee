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

static void do_fastcgi (tipidee_rql const *rql, int fd, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen, tain const *deadline, char const *fn)
{
  char buf[4096] ;
  buffer b = BUFFER_INIT(&buffer_write, fd, buf, 4096) ;
  _exit(0) ;
}

static void fastcgi_unix (tipidee_rql const *rql, char const *socketpath, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  tain deadline ;
  int fd = ipc_stream_nbcoe() ;
  if (fd == -1) die500sys(rql, 111, docroot, "create socket") ;
  tain_add_g(&deadline, &g.readtto) ;
  if (!ipc_timed_connect_g(fd, socketpath, &deadline)) die500sys(rql, 111, docroot, "connect to ", socketpath) ;
  do_fastcgi(rql, fd, sub, docroot, hdr, body, bodylen, &deadline, socketpath) ;
  fd_close(fd) ;
}

static inline void fastcgi_tcp (tipidee_rql const *rql, char const *ip, uint16_t port, int is6, char const *sub, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
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
  do_fastcgi(rql, fd, sub, docroot, hdr, body, bodylen, &deadline, fmt) ;
  fd_close(fd) ;
}

void fastcgi (tipidee_rql const *rql, tipidee_redirection const *rd, char const *docroot, tipidee_headers const *hdr, char const *body, size_t bodylen)
{
  if (rd->flags & TIPIDEE_REDIR_ISINET)
    fastcgi_tcp(rql, rd->addr, rd->port, !!(rd->flags & TIPIDEE_REDIR_ISV6), rd->sub, docroot, hdr, body, bodylen) ;
  else
    fastcgi_unix(rql, rd->addr, rd->sub, docroot, hdr, body, bodylen) ;
}
