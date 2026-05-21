/* ISC license. */

#include <string.h>

#include <skalibs/uint16.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_beginrequest_body_pack (char *s, fcgi_beginrequest_body const *bd)
{
  uint16_pack_big(s, bd->role) ; s += 2 ;
  *s++ = bd->flags ;
  memset(s, 0, 5) ;
}
