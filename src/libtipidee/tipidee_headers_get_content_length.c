/* ISC license. */

#include <stddef.h>
#include <limits.h>

#include <skalibs/types.h>

#include <tipidee/headers.h>

ssize_t tipidee_headers_get_content_length (tipidee_headers const *hdr)
{
  size_t n ;
  char const *x = tipidee_headers_search(hdr, "Content-Length") ;
  if (!x) return 0 ;
  if (!size0_scan(x, &n) || n > SSIZE_MAX) return -1 ;
  return n ;
}
