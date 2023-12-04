/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_file_G (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, struct stat const *st, char const *ct, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_file(b, rql, status, reason, st, ct, rhdr, rhdrn, options, &wstamp) ;
}
