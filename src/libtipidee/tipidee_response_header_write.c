/* ISC license. */

#include <stddef.h>

#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_header_write (buffer *b, tipidee_response_header const *hdr, uint32_t n)
{
  size_t m = 0 ;
  for (uint32_t i = 0 ; i < n ; i++)
  {
    if (!hdr[i].value) continue ;
    m += buffer_putsnoflush(b, hdr[i].key) ;
    m += buffer_putnoflush(b, ": ", 2) ;
    m += buffer_putsnoflush(b, hdr[i].value) ;
    m += buffer_putnoflush(b, "\r\n", 2) ;
  }
  return m ;
}
