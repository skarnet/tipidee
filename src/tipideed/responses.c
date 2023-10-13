/* ISC license. */

#include <unistd.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/rql.h>
#include <tipidee/log.h>
#include <tipidee/response.h>

#include "tipideed-internal.h"

void response_error (tipidee_rql const *rql, unsigned int status, char const *rsl, char const *text, uint32_t options)
{
  tain deadline ;
  tipidee_response_error(buffer_1, rql, status, rsl, text, options & 1 || !g.cont) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!(options & 2)) tipidee_log_answer(g.logv, rql, status, 0) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_and_exit (tipidee_rql const *rql, unsigned int status, char const *rsl, char const *text, uint32_t options)
{
  response_error(rql, status, rsl, text, options | 1) ;
  tipidee_log_exit(g.logv, 0) ;
  _exit(0) ;
}

void response_error_and_die (tipidee_rql const *rql, int e, unsigned int status, char const *rsl, char const *text, char const *const *v, unsigned int n, uint32_t options)
{
  response_error(rql, status, rsl, text, options | 1) ;
  if (options & 1) strerr_dievsys(e, v, n) ;
  else strerr_diev(e, v, n) ;
}

void exit_405 (tipidee_rql const *rql, uint32_t options)
{
  tain deadline ;
  tipidee_response_status(buffer_1, rql, 405, "Method Not Allowed") ;
  tipidee_response_header_common_put_g(buffer_1, 1) ;
  buffer_putsnoflush(buffer_1, "Allow: GET, HEAD") ;
  if (options & 1) buffer_putsnoflush(buffer_1, ", POST") ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  if (!(options & 2)) tipidee_log_answer(g.logv, rql, 405, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  tipidee_log_exit(g.logv, 0) ;
  _exit(0) ;
}

void respond_30x (tipidee_rql const *rql, tipidee_redirection const *rd)
{
  static unsigned int const status[4] = { 307, 308, 302, 301 } ;
  static char const *const reason[4] = { "Temporary Redirect", "Permanent Redirect", "Found", "Moved Permanently" } ;
  tain deadline ;
  tipidee_response_status(buffer_1, rql, status[rd->type], reason[rd->type]) ;
  tipidee_response_header_common_put_g(buffer_1, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Length: 0\r\nLocation: ") ;
  buffer_putsnoflush(buffer_1, rd->location) ;
  if (rd->sub) buffer_putsnoflush(buffer_1, rd->sub) ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  tipidee_log_answer(g.logv, rql, status[rd->type], 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}
