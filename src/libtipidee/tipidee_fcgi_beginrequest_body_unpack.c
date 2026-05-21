/* ISC license. */

#include <string.h>

#include <skalibs/uint16.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_beginrequest_body_unpack (char const *s, fcgi_beginrequest_body *bd)
{
  uint16_unpack_big(s, &bd->role) ; s += 2 ;
  bd->flags = *s++ ;
  memcpy((char *)bd->reserved, s, 5) ; s += 5 ;
}
