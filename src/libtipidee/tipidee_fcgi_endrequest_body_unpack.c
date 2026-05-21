/* ISC license. */

#include <string.h>

#include <skalibs/uint32.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_endrequest_body_unpack (char const *s, fcgi_endrequest_body *bd)
{
  uint32_unpack_big(s, &bd->appstatus) ; s += 4 ;
  bd->protostatus = *s++ ;
  memcpy((char *)bd->reserved, s, 3) ; s += 3 ;
}
