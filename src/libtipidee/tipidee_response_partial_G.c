/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_partial_G (buffer *b, tipidee_rql const *rql, struct stat const *st, uint64_t start, uint64_t len, char const *ct, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_partial(b, rql, st, start, len, ct, rhdr, rhdrn, options, &wstamp) ;
}
