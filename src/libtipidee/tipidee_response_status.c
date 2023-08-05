/* ISC license. */

#include <skalibs/types.h>
#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_status (buffer *b, tipidee_rql const *rql, unsigned int status, char const *line)
{
  size_t n = 0 ;
  char fmt[UINT_FMT] ;
  n += buffer_putnoflush(b, "HTTP/", 5) ;
  n += buffer_putnoflush(b, fmt, uint_fmt(fmt, rql->http_major ? rql->http_major : 1)) ;
  n += buffer_putnoflush(b, ".", 1) ;
  n += buffer_putnoflush(b, fmt, uint_fmt(fmt, rql->http_major ? rql->http_minor : 1)) ;
  n += buffer_putnoflush(b, " ", 1) ;
  if (status)
  {
    char fmt[UINT_FMT] ;
    size_t m = uint_fmt(fmt, status) ;
    n += buffer_putnoflush(b, fmt, m) ;
    n += buffer_putnoflush(b, " ", 1) ;
  }
  n += buffer_putsnoflush(b, line) ;
  n += buffer_putnoflush(b, "\r\n", 2) ;
  return n ;
}
