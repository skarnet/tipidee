/* ISC license. */

#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/stralloc.h>

#include <tipidee/fcgi.h>

static int encode (stralloc *sa, size_t n)
{
  if (n <= 0x7f)
  {
    char c = n ;
    return stralloc_catb(sa, &c, 1) ;
  }
  else if (n < 0x7ffffffful)
  {
    char pack[4] ;
    uint32_pack_big(pack, n | 0x80000000) ;
    return stralloc_catb(sa, pack, 4) ;
  }
  else return (errno = EOVERFLOW, 0) ;
}

int tipidee_fcgi_addenvb (stralloc *sa, char const *key, size_t keylen, char const *val, size_t vallen)
{
  size_t start = sa->len ;
  if (!encode(sa, keylen)) return 0 ;
  if (!encode(sa, vallen)
    || !stralloc_catb(sa, key, keylen)
    || !stralloc_catb(sa, val, vallen)) goto err ;
  if (sa->len > 0x7fff) goto err0 ;
  return 1 ;

 err0:
  errno = E2BIG ;
 err:
  sa->len = start ;
  return 0 ;
}
