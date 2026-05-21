/* ISC license. */

#include <tipidee/fcgi.h>

void tipidee_fcgi_beginrequest_record_pack (char *s, fcgi_beginrequest_record const *rec)
{
  tipidee_fcgi_header_pack(s, &rec->header) ; s += 8 ;
  tipidee_fcgi_beginrequest_body_pack(s, &rec->body) ;
}
