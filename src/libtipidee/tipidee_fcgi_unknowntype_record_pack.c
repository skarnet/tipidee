/* ISC license. */

#include <tipidee/fcgi.h>

void tipidee_fcgi_unknowntype_record_pack (char *s, fcgi_unknowntype_record const *rec)
{
  tipidee_fcgi_header_pack(s, &rec->header) ; s += 8 ;
  tipidee_fcgi_unknowntype_body_pack(s, &rec->body) ;
}
