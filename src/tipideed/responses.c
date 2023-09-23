/* ISC license. */

#include <unistd.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/rql.h>
#include <tipidee/response.h>

#include "tipideed-internal.h"

void response_error (tipidee_rql const *rql, char const *rsl, char const *text, int doclose)
{
  tain deadline ;
  char ans[4] = "???" ;
  memcpy(ans, rsl, 3) ;
  tipidee_response_error(buffer_1, rql, rsl, text, doclose || !g.cont) ;
  tain_add_g(&deadline, &g.writetto) ;
  log_response(ans, 0) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_and_exit (tipidee_rql const *rql, char const *rsl, char const *text)
{
  response_error(rql, rsl, text, 1) ;
  log_and_exit(0) ;
}

void response_error_and_die (tipidee_rql const *rql, int e, char const *rsl, char const *text, char const *const *v, unsigned int n, int dosys)
{
  response_error(rql, rsl, text, 1) ;
  if (dosys) strerr_dievsys(e, v, n) ;
  else strerr_diev(e, v, n) ;
}

void exit_405 (tipidee_rql const *rql, uint32_t options)
{
  tain deadline ;
  tipidee_response_status_line(buffer_1, rql, "405 Method Not Allowed") ;
  tipidee_response_header_common_put_g(buffer_1, 1) ;
  buffer_putsnoflush(buffer_1, "Allow: GET, HEAD") ;
  if (options & 1) buffer_putsnoflush(buffer_1, ", POST") ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  log_and_exit(0) ;
}

void respond_30x (tipidee_rql const *rql, tipidee_redirection const *rd)
{
  static char const *rsl[4] = { "307 Temporary Redirect", "308 Permanent Redirect", "302 Found", "301 Moved Permanently" } ;
  tain deadline ;
  char ans[4] = "30x" ;
  memcpy(ans, rsl[rd->type], 3) ;
  tipidee_response_status_line(buffer_1, rql, rsl[rd->type]) ;
  tipidee_response_header_common_put_g(buffer_1, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Length: 0\r\nLocation: ") ;
  buffer_putsnoflush(buffer_1, rd->location) ;
  if (rd->sub) buffer_putsnoflush(buffer_1, rd->sub) ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  tain_add_g(&deadline, &g.writetto) ;
  log_response(ans, 0) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}
