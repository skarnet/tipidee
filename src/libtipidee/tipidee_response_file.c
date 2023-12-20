/* ISC license. */

#include <stddef.h>

#include <skalibs/uint64.h>
#include <skalibs/buffer.h>

#include <tipidee/response.h>

size_t tipidee_response_file (buffer *b, tipidee_rql const *rql, unsigned int status, char const *reason, struct stat const *st, uint64_t cl, char const *ct, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  tipidee_response_header v[2] = { { .key = "Content-Type", .value = ct, .options = 0 }, { .key = "Content-Length", .value = fmt, .options = 0 } } ;
  size_t n = tipidee_response_status(b, rql, status, reason) ;
  n += tipidee_response_header_writeall(b, rhdr, rhdrn, options & 1, stamp) ;
  if (options & 2)
  {
    size_t l = tipidee_response_header_lastmodified(fmt, 128, st) ;
    if (l) n += buffer_putnoflush(b, fmt, l) ;
  }
  fmt[uint64_fmt(fmt, cl)] = 0 ;
  n += tipidee_response_header_write(b, v, 2) ;
  return n ;
}
