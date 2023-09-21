/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <tipidee/response.h>

size_t tipidee_response_header_lastmodified (char *s, size_t max, struct stat const *st)
{
  tain t ;
  size_t l ;
  if (max < 17) return (errno = ENOBUFS, 0) ;
  if (!tain_from_timespec(&t, &st->st_mtim)) return 0 ;
  memcpy(s, "Last-Modified: ", 15) ;
  l = tipidee_response_header_date_fmt(s + 15, max - 15, &t) ;
  if (!l) return 0 ;
  if (l + 17 > max) return (errno = ENOBUFS, 0) ;
  l += 15 ;
  s[l++] = '\r' ; s[l++] = '\n' ;
  return l ;
}
