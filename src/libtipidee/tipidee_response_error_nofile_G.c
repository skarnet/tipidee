/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_error_nofile_G (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, tipidee_response_header const *rhdr, uint32_t rhdrn, tipidee_response_header const *plus, uint32_t plusn, uint32_t options)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_error_nofile(b, rql, status, reason, text, rhdr, rhdrn, plus, plusn, options, &wstamp) ;
}
