/* ISC license. */

#include <tipidee/fcgi.h>

void tipidee_fcgi_beginrequest_record_unpack (char const *s, fcgi_beginrequest_record *rec)
{
  tipidee_fcgi_header_unpack(s, &rec->header) ; s += 8 ;
  tipidee_fcgi_beginrequest_body_unpack(s, &rec->body) ;
}
