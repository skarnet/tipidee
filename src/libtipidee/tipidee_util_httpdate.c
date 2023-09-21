/* ISC license. */

#include <errno.h>
#include <time.h>

#include <skalibs/djbtime.h>

#include <tipidee/util.h>

int tipidee_util_httpdate (char const *s, tain *stamp)
{
  struct tm tm ;
  if (strptime(s, "%a, %d %b %Y %T GMT", &tm)) goto got ;
  if (strptime(s, "%a, %d-%b-%y %T GMT", &tm)) goto got ;
  if (strptime(s, "%a %b %d %T %Y", &tm)) goto got ;
  return (errno = EINVAL, 0) ;

 got:
  if (!tai_from_localtm(tain_secp(stamp), &tm)) return 0 ;
  stamp->nano = 0 ;
  return 1 ;
}
