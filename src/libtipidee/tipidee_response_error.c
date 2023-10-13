/* ISC license. */

#include <stddef.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>

#include <tipidee/method.h>
#include <tipidee/rql.h>
#include <tipidee/response.h>

size_t tipidee_response_error (buffer *b, tipidee_rql const *rql, unsigned int status, char const *rsl, char const *text, uint32_t options)
{
  size_t n = 0 ;
  static char const txt1[] = "<html>\n<head><title>" ;
  static char const txt2[] = "</title></head>\n<body>\n<h1> " ;
  static char const txt3[] = " </h1>\n<p>\n" ;
  static char const txt4[] = "\n</p>\n</body>\n</html>\n" ;
  n += tipidee_response_status(b, rql, status, rsl) ;
  n += tipidee_response_header_common_put_g(buffer_1, options) ;
  if (!(options & 2))
  {
    char fmt[SIZE_FMT] ;
    n += buffer_putsnoflush(buffer_1, "Content-Type: text/html; charset=UTF-8\r\n") ;
    n += buffer_putsnoflush(buffer_1, "Content-Length: ") ;
    n += buffer_putnoflush(buffer_1, fmt, size_fmt(fmt, sizeof(txt1) + sizeof(txt2) + sizeof(txt3) + sizeof(txt4) - 4 + 2 * strlen(rsl) + strlen(text))) ;
    n += buffer_putnoflush(buffer_1, "\r\n", 2) ;
  }
  n += buffer_putnoflush(buffer_1, "\r\n", 2) ;
  if (rql->m != TIPIDEE_METHOD_HEAD)
  {
    n += buffer_putsnoflush(buffer_1, txt1) ;
    n += buffer_putsnoflush(buffer_1, rsl) ;
    n += buffer_putsnoflush(buffer_1, txt2) ;
    n += buffer_putsnoflush(buffer_1, rsl) ;
    n += buffer_putsnoflush(buffer_1, txt3) ;
    n += buffer_putsnoflush(buffer_1, text) ;
    n += buffer_putsnoflush(buffer_1, txt4) ;
  }
  return n ;
}
