/* ISC license. */

#include <string.h>

#include <tipidee/method.h>

tipidee_method tipidee_method_tonum (char const *s)
{
  tipidee_method_conv const *p = tipidee_method_conv_table ;
  for (; p->str ; p++) if (!strcmp(s, p->str)) break ;
  return p->num ;
}
