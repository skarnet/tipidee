/* ISC license. */

#include <string.h>

#include <skalibs/uint32.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_endrequest_body_pack (char *s, fcgi_endrequest_body const *bd)
{
  uint32_pack_big(s, bd->appstatus) ; s += 4 ;
  *s++ = bd->protostatus ;
  memset(s, 0, 3) ;
}
