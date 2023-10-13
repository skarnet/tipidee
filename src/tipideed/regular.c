/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <errno.h>

#include <skalibs/uint64.h>
#include <skalibs/stat.h>
#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/djbunix.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/method.h>
#include <tipidee/response.h>
#include <tipidee/log.h>
#include "tipideed-internal.h"

int respond_regular (tipidee_rql const *rql, char const *fn, struct stat const *st, tipidee_resattr const *ra)
{
  tain deadline ;
  char fmt[128] ;
  size_t n = tipidee_response_status(buffer_1, rql, 200, "OK") ;
  n += tipidee_response_header_common_put_g(buffer_1, !g.cont) ;
  {
    size_t l = tipidee_response_header_lastmodified(fmt, 128, st) ;
    if (l) n += buffer_putnoflush(buffer_1, fmt, l) ;
  }
  n += buffer_putsnoflush(buffer_1, "Content-Type: ") ;
  n += buffer_putsnoflush(buffer_1, ra->content_type) ;
  n += buffer_putsnoflush(buffer_1, "\r\nContent-Length: ") ;
  fmt[uint64_fmt(fmt, st->st_size)] = 0 ;
  n += buffer_putsnoflush(buffer_1, fmt) ;
  n += buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  if (rql->m == TIPIDEE_METHOD_HEAD)
  {
    tipidee_log_answer(g.logv, rql, 200, st->st_size) ;
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
    tipidee_log_answer(g.logv, rql, 200, st->st_size) ;
    send_file(fd, st->st_size, fn) ;
    fd_close(fd) ;
  }
  return 0 ;
}

int respond_304 (tipidee_rql const *rql, char const *fn, struct stat const *st)
{
  tain deadline ;
  char fmt[128] ;
  size_t n = tipidee_response_status(buffer_1, rql, 304, "Not Modified") ;
  n += tipidee_response_header_common_put_g(buffer_1, !g.cont) ;
  {
    size_t l = tipidee_response_header_lastmodified(fmt, 128, st) ;
    if (l) n += buffer_putnoflush(buffer_1, fmt, l) ;
  }
  n += buffer_putnoflush(buffer_1, "\r\n", 2) ;
  tipidee_log_answer(g.logv, rql, 304, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  return 0 ;
}
