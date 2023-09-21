/* ISC license. */

#include <stdint.h>

#include <skalibs/buffer.h>

#include <tipidee/config.h>
#include <tipidee/response.h>

size_t tipidee_response_header_common_put (buffer *b, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t m = buffer_putnoflush(b, fmt, tipidee_response_header_date(fmt, 128, stamp)) ;
  for (tipidee_response_header_builtin const *p = tipidee_response_header_builtin_table ; p->key ; p++)
  {
    m += buffer_putsnoflush(b, p->key) ;
    m += buffer_putnoflush(b, ": ", 2) ;
    m += buffer_putsnoflush(b, p->value) ;
    m += buffer_putnoflush(b, "\r\n", 2) ;
  }
  if (options & 1) m += buffer_putsnoflush(b, "Connection: close\r\n") ;
  return m ;
}
