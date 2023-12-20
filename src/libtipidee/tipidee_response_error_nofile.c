/* ISC license. */

#include <string.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>

#include <tipidee/method.h>
#include <tipidee/response.h>

size_t tipidee_response_error_nofile (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, tipidee_response_header const *rhdr, uint32_t rhdrn, tipidee_response_header const *plus, uint32_t plusn, uint32_t options, tain const *stamp)
{
  static char const txt1[] = "<html>\n<head><title>" ;
  static char const txt2[] = "</title></head>\n<body>\n<h1> " ;
  static char const txt3[] = " </h1>\n<p>\n" ;
  static char const txt4[] = "\n</p>\n</body>\n</html>\n" ;
  char fmt[SIZE_FMT] ;
  tipidee_response_header v[2] = { { .key = "Content-Type", .value = "text/html; charset=UTF-8", .options = 0 }, { .key = "Content-Length", .value = fmt, .options = 0 } } ;
  size_t n = tipidee_response_status(b, rql, status, reason) ;
  n += tipidee_response_header_writeall(b, rhdr, rhdrn, options, stamp) ;
  if (plusn) n += tipidee_response_header_write(b, plus, plusn) ;
  fmt[size_fmt(fmt, text ? sizeof(txt1) + sizeof(txt2) + sizeof(txt3) + sizeof(txt4) - 4 + 2 * strlen(reason) + strlen(text) : 0)] = 0 ;
  n += tipidee_response_header_write(b, v, 2) ;

  n += buffer_putnoflush(b, "\r\n", 2) ;
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
