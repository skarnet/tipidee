/* ISC license. */

#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_header_end (buffer *b)
{
  return buffer_put(b, "\r\n", 2) ;
}
