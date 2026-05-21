/* ISC license. */

#include <skalibs/uint16.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_header_pack (char *s, fcgi_header const *hdr)
{
  *s++ = hdr->version ;
  *s++ = hdr->type ;
  uint16_pack_big(s, hdr->requestid) ; s += 2 ;
  uint16_pack_big(s, hdr->len) ; s += 2 ;
  *s++ = hdr->padlen ;
  *s++ = 0 ;
}
