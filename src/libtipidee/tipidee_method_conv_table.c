/* ISC license. */

#include <tipidee/method.h>

static tipidee_method_conv const table[] =
{
  { .num = TIPIDEE_METHOD_GET, .str = "GET" },
  { .num = TIPIDEE_METHOD_HEAD, .str = "HEAD" },
  { .num = TIPIDEE_METHOD_OPTIONS, .str = "OPTIONS" },
  { .num = TIPIDEE_METHOD_POST, .str = "POST" },
  { .num = TIPIDEE_METHOD_PUT, .str = "PUT" },
  { .num = TIPIDEE_METHOD_DELETE, .str = "DELETE" },
  { .num = TIPIDEE_METHOD_TRACE, .str = "TRACE" },
  { .num = TIPIDEE_METHOD_CONNECT, .str = "CONNECT" },
  { .num = TIPIDEE_METHOD_PRI, .str = "PRI" },
  { .num = TIPIDEE_METHOD_UNKNOWN, .str = 0 }
} ;

tipidee_method_conv const *tipidee_method_conv_table = table ;
