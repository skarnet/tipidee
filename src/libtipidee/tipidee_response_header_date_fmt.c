/* ISC license. */

#include <time.h>

#include <skalibs/tai.h>
#include <skalibs/djbtime.h>

#include <tipidee/response.h>

size_t tipidee_response_header_date_fmt (char *s, size_t max, tain const *stamp)
{
  struct tm tm ;
  if (!localtm_from_tai(&tm, tain_secp(stamp), 0)) return 0 ;
  return strftime(s, max, "%a, %d %b %Y %T GMT", &tm) ;
}
