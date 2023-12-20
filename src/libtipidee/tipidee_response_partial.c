/* ISC license. */

#include <stddef.h>

#include <skalibs/uint64.h>
#include <skalibs/buffer.h>

#include <tipidee/util.h>
#include <tipidee/response.h>

size_t tipidee_response_partial (buffer *b, tipidee_rql const *rql, struct stat const *st, uint64_t start, uint64_t len, char const *ct, tipidee_response_header const *rhdr, uint32_t rhdrn, uint32_t options, tain const *stamp)
{
  tipidee_defaulttext dt ;
  size_t n ;
  char fmt[UINT64_FMT] ;
  if (!tipidee_util_defaulttext(206, &dt)) return 0 ;
  n = tipidee_response_file(b, rql, 206, dt.reason, st, len, ct, rhdr, rhdrn, options, stamp) ;
  if (len)
  {
    n += buffer_putsnoflush(b, "Content-Range: bytes ") ;
    n += buffer_putnoflush(b, fmt, uint64_fmt(fmt, start)) ;
    n += buffer_putnoflush(b, "-", 1) ;
    n += buffer_putnoflush(b, fmt, uint64_fmt(fmt, start + len - 1)) ;
    n += buffer_putnoflush(b, "/", 1) ;
    n += buffer_putnoflush(b, fmt, uint64_fmt(fmt, st->st_size)) ;
    n += buffer_putnoflush(b, "\r\n", 2) ;
  }
  return n ;
}
