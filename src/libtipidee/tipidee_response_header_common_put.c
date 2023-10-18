/* ISC license. */

#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_header_common_put (buffer *b, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t n = buffer_putnoflush(b, fmt, tipidee_response_header_date(fmt, 128, stamp)) ;
  for (tipidee_response_header_builtin const *p = tipidee_response_header_builtin_table ; p->key ; p++)
  {
    n += buffer_putsnoflush(b, p->key) ;
    n += buffer_putnoflush(b, ": ", 2) ;
    n += buffer_putsnoflush(b, p->value) ;
    n += buffer_putnoflush(b, "\r\n", 2) ;
  }
  if (options & 1) n += buffer_putsnoflush(b, "Connection: close\r\n") ;
  return n ;
}
