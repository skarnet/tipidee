/* ISC license. */

#include <string.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>

#include <tipidee/method.h>
#include <tipidee/response.h>

size_t tipidee_response_error_nofile (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  static char const txt1[] = "<html>\n<head><title>" ;
  static char const txt2[] = "</title></head>\n<body>\n<h1> " ;
  static char const txt3[] = " </h1>\n<p>\n" ;
  static char const txt4[] = "\n</p>\n</body>\n</html>\n" ;
  char fmt[SIZE_FMT] ;
  size_t n = tipidee_response_status(b, rql, status, reason) ;
  n += tipidee_response_header_writeall(b, rhdr, rhdrn, options, stamp) ;
  n += buffer_putsnoflush(b, "Content-Type: text/html; charset=UTF-8\r\n") ;
  n += buffer_putsnoflush(b, "Content-Length: ") ;
  n += buffer_putnoflush(b, fmt, size_fmt(fmt, text ? sizeof(txt1) + sizeof(txt2) + sizeof(txt3) + sizeof(txt4) - 4 + 2 * strlen(reason) + strlen(text) : 0)) ;
  n += buffer_putnoflush(b, "\r\n\r\n", 4) ;
  if (text && rql->m != TIPIDEE_METHOD_HEAD)
  {
    n += buffer_putsnoflush(b, txt1) ;
    n += buffer_putsnoflush(b, reason) ;
    n += buffer_putsnoflush(b, txt2) ;
    n += buffer_putsnoflush(b, reason) ;
    n += buffer_putsnoflush(b, txt3) ;
    n += buffer_putsnoflush(b, text) ;
    n += buffer_putsnoflush(b, txt4) ;
  }
  return n ;
}
