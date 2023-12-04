/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_header_writeall_G (buffer *b, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_header_writeall(b, rhdr, rhdrn, options, &wstamp) ;
}
