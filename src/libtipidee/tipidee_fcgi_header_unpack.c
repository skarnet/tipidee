/* ISC license. */

#include <skalibs/uint16.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_header_unpack (char const *s, fcgi_header *hdr)
{
  hdr->version = *s++ ;
  hdr->type = *s++ ;
  uint16_unpack_big(s, &hdr->requestid) ; s += 2 ;
  uint16_unpack_big(s, &hdr->len) ; s += 2 ;
  hdr->padlen = *s++ ;
}
