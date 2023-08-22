/* ISC license. */

#include <errno.h>

#include <skalibs/uint64.h>
#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/method.h>
#include <tipidee/response.h>
#include "tipideed-internal.h"

int respond_regular (tipidee_rql const *rql, char const *fn, uint64_t size, tipidee_resattr const *ra)
{
  tain deadline ;
  size_t n = tipidee_response_status_line(buffer_1, rql, "200 OK") ;
  n += tipidee_response_header_common_put_g(buffer_1, !g.cont) ;
  n += buffer_putsnoflush(buffer_1, "Content-Type: ") ;
  n += buffer_putsnoflush(buffer_1, ra->content_type) ;
  n += buffer_putsnoflush(buffer_1, "\r\nContent-Length: ") ;
  {
    char fmt[UINT64_FMT] ;
    fmt[uint64_fmt(fmt, size)] = 0 ;
    n += buffer_putsnoflush(buffer_1, fmt) ;
    log_regular(fn, fmt, rql->m == TIPIDEE_METHOD_HEAD, ra->content_type) ;
  }
  n += buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  if (rql->m == TIPIDEE_METHOD_HEAD)
  {
    tain_add_g(&deadline, &g.writetto) ;
    if (!buffer_timed_flush_g(buffer_1, &deadline))
      strerr_diefu1sys(111, "write to stdout") ;
  }
  else
  {
    int fd = open_read(fn) ;
    if (fd == -1)
    {
      buffer_unput(buffer_1, n) ;
      if (errno == EACCES)
      {
        respond_403(rql) ;
        return 0 ;
      }
      else die500sys(rql, 111, "open ", fn) ;
    }
    send_file(fd, size, fn) ;
    fd_close(fd) ;
  }
  return 0 ;
}
