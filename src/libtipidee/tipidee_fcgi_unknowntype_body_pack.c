/* ISC license. */

#include <string.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_unknowntype_body_pack (char *s, fcgi_unknowntype_body const *bd)
{
  *s++ = bd->type ;
  memset(s, 0, 7) ;
}
