/* ISC license. */

#include <stddef.h>

#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_header_writeall (buffer *b, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t m = buffer_putnoflush(b, fmt, tipidee_response_header_date(fmt, 128, stamp)) ;
  if (options & 1) m += buffer_putsnoflush(b, "Connection: close\r\n") ;
  m += tipidee_response_header_write(b, rhdr, rhdrn) ;
  return m ;
}
