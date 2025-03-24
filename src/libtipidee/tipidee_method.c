/* ISC license. */

#include <string.h>
#include <stdlib.h>

#include <tipidee/method.h>

struct blah_s
{
  tipidee_method num ;
  char const *str ;
} ;

static struct blah_s const table[] =
{
  { .num = TIPIDEE_METHOD_CONNECT, .str = "CONNECT" },
  { .num = TIPIDEE_METHOD_DELETE, .str = "DELETE" },
  { .num = TIPIDEE_METHOD_GET, .str = "GET" },
  { .num = TIPIDEE_METHOD_HEAD, .str = "HEAD" },
  { .num = TIPIDEE_METHOD_OPTIONS, .str = "OPTIONS" },
  { .num = TIPIDEE_METHOD_PATCH, .str = "PATCH" },
  { .num = TIPIDEE_METHOD_POST, .str = "POST" },
  { .num = TIPIDEE_METHOD_PRI, .str = "PRI" },
  { .num = TIPIDEE_METHOD_PUT, .str = "PUT" },
  { .num = TIPIDEE_METHOD_TRACE, .str = "TRACE" },
} ;

static int blah_cmp (void const *key, void const *el)
{
  return strcmp((char const *)key, ((struct blah_s const *)el)->str) ;
}

tipidee_method tipidee_method_tonum (char const *s)
{
  struct blah_s const *p = bsearch(
    s,
    table,
    sizeof(table) / sizeof(struct blah_s),
    sizeof(struct blah_s),
    &blah_cmp) ;
  return p ? p->num : TIPIDEE_METHOD_UNKNOWN ;
}

char const *tipidee_method_tostr (tipidee_method m)
{
  return m < TIPIDEE_METHOD_UNKNOWN ? table[m].str : 0 ;
}
