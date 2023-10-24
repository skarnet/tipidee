/* ISC license. */

#include <string.h>

#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/log.h>
#include <tipidee/response.h>
#include "tipideed-internal.h"

int respond_options (tipidee_rql const *rql, uint32_t flags)
{
  tain deadline ;
  tipidee_response_status(buffer_1, rql, 200, "OK") ;
  tipidee_response_header_writeall_g(buffer_1, g.rhdr, g.rhdrn, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Length: 0\r\nAllow: GET, HEAD") ;
  if (flags & 1) buffer_putsnoflush(buffer_1, ", POST") ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  if (flags & 2) tipidee_log_answer(g.logv, rql, 200, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  return 0 ;
}
