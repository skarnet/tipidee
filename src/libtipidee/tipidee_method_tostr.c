/* ISC license. */

#include <tipidee/method.h>

char const *tipidee_method_tostr (tipidee_method m)
{
  return m < TIPIDEE_METHOD_UNKNOWN ? tipidee_method_conv_table[m].str : 0 ;
}
