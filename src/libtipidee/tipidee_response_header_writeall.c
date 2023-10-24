/* ISC license. */

#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_header_writeall (buffer *b, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t m = buffer_putnoflush(b, fmt, tipidee_response_header_date(fmt, 128, stamp)) ;
  if (options & 1) m += buffer_putsnoflush(b, "Connection: close\r\n") ;
  for (uint32_t i = 0 ; i < rhdrn ; i++)
  {
    m += buffer_putsnoflush(b, rhdr[i].key) ;
    m += buffer_putnoflush(b, ": ", 2) ;
    m += buffer_putsnoflush(b, rhdr[i].value) ;
    m += buffer_putnoflush(b, "\r\n", 2) ;
  }
  return m ;
}
