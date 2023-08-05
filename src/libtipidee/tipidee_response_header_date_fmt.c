/* ISC license. */

#include <string.h>
#include <time.h>

#include <skalibs/tai.h>
#include <skalibs/djbtime.h>

#include <tipidee/response.h>

size_t tipidee_response_header_date_fmt (char *s, size_t max, tain const *stamp)
{
  size_t m = 0, l ;
  struct tm tm ;
  if (m + 6 > max) return 0 ;
  if (!localtm_from_tai(&tm, tain_secp(stamp), 0)) return 0 ;
  memcpy(s, "Date: ", 6) ; m += 6 ;
  l = strftime(s + m, max - m, "%a, %d %b %Y %T GMT", &tm) ;
  if (!l) return 0 ;
  m += l ;
  if (m + 2 > max) return 0 ;
  s[m++] = '\r' ; s[m++] = '\n' ;
  return m ;
}
