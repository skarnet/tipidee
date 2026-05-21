/* ISC license. */

#include <string.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_unknowntype_body_unpack (char const *s, fcgi_unknowntype_body *bd)
{
  bd->type = *s++ ;
  memcpy((char *)bd->reserved, s, 7) ; s += 7 ;
}
