/* ISC license. */

#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/log.h>
#include <tipidee/util.h>
#include <tipidee/response.h>
#include "tipideed-internal.h"

void respond_30x (tipidee_rql const *rql, tipidee_redirection const *rd)
{
  static unsigned int const status[4] = { 307, 308, 302, 301 } ;
  tipidee_defaulttext dt ;
  tain deadline ;
  tipidee_util_defaulttext(status[rd->type], &dt) ;
  tipidee_response_status(buffer_1, rql, status[rd->type], dt.reason) ;
  tipidee_response_header_writeall_G(buffer_1, g.rhdr, g.rhdrn, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Length: 0\r\nLocation: ") ;
  buffer_putsnoflush(buffer_1, rd->location) ;
  if (rd->sub) buffer_putsnoflush(buffer_1, rd->sub) ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  tipidee_log_answer(g.logv, rql, status[rd->type], 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}
