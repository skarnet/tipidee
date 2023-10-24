/* ISC license. */

#include <stddef.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>

#include <tipidee/conf.h>
#include <tipidee/method.h>
#include <tipidee/rql.h>
#include <tipidee/response.h>
#include <tipidee/util.h>

size_t tipidee_response_file (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, struct stat const *st, char const *ct, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t n = tipidee_response_status(b, rql, status, reason) ;
  n += tipidee_response_header_writeall(b, rhdr, rhdrn, options & 1, stamp) ;
  if (options & 2)
  {
    size_t l = tipidee_response_header_lastmodified(fmt, 128, st) ;
    if (l) n += buffer_putnoflush(b, fmt, l) ;
  }
  n += buffer_putsnoflush(b, "Content-Type: ") ;
  n += buffer_putsnoflush(b, ct) ;
  n += buffer_putsnoflush(b, "\r\nContent-Length: ") ;
  fmt[uint64_fmt(fmt, st->st_size)] = 0 ;
  n += buffer_putsnoflush(b, fmt) ;
  n += buffer_putnoflush(b, "\r\n\r\n", 4) ;
  return n ;
}
