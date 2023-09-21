/* ISC license. */

#include <errno.h>
#include <string.h>

#include <tipidee/response.h>

size_t tipidee_response_header_date (char *s, size_t max, tain const *stamp)
{
  size_t l ;
  if (max < 8) return (errno = ENOBUFS, 0) ;
  memcpy(s, "Date: ", 6) ;
  l = tipidee_response_header_date_fmt(s + 6, max - 6, stamp) ;
  if (!l) return 0 ;
  if (l + 8 > max) return (errno = ENOBUFS, 0) ;
  l += 6 ;
  s[l++] = '\r' ; s[l++] = '\n' ;
  return l ;
}
