/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_header_writemerge_G (buffer *b, tipidee_response_header const *rhdr, uint32_t rhdrn, tipidee_headers const *hdr, uint32_t options)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_header_writemerge(b, rhdr, rhdrn, hdr, options, &wstamp) ;
}
