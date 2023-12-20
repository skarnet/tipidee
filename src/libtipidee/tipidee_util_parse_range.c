/* ISC license. */

#include <string.h>

#include <skalibs/uint64.h>

#include <tipidee/util.h>

int tipidee_util_parse_range (char const *s, off_t max, uint64_t *start, uint64_t *len)
{
  if (strncmp(s, "bytes=", 6)) return -1 ;
  s += 6 ;
  if (*s == '-')
  {
    uint64_t n ;
    size_t m = uint64_scan(++s, &n) ;
    if (!m) return -1 ;
    s += m ;
    if (*s && *s != ',') return -1 ;
    if (n > max) return -1 ;
    *start = max - n ;
    *len = n ;
    return 1 ;
  }
  else
  {
    uint64_t beg ;
    uint64_t n ;
    size_t m = uint64_scan(s, &beg) ;
    if (!m) return -1 ;
    s += m ;
    if (*s++ != '-') return -1 ;
    if (beg >= max) return -1 ;
    if (!*s || *s == ',')
    {
      *start = beg ;
      *len = max - beg ;
      return 1 ;
    }
    m = uint64_scan(s, &n) ;
    if (!m) return -1 ;
    s += m ;
    if (*s && *s != ',') return -1 ;
    if (n >= max || n < beg) return -1 ;
    *start = beg ;
    *len = n + 1 - beg ;
    return 1 ;
  }
}
