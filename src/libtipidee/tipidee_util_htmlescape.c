/* ISC license. */

#include <stdlib.h>

#include <tipidee/util.h>

struct htmlescape_s
{
  char c ;
  char const *code ;
} ;

static struct htmlescape_s const table[] =
{
  { .c = '\"', .code = "&quot;" },
  { .c = '&', .code = "&amp;" },
  { .c = '\'', .code = "&#39;" },
  { .c = '<', .code = "&lt;" },
  { .c = '>', .code = "&gt;" },
} ;

static int htmlescape_cmp (void const *a, void const *b)
{
  char aa = *((char const *)a) ;
  char bb = ((struct htmlescape_s const *)b)->c ;
  return aa < bb ? -1 : aa > bb ;
}

char const *tipidee_util_htmlescape (char const *s)
{
  struct htmlescape_s const *p = bsearch(
    s,
    table,
    sizeof(table) / sizeof(struct htmlescape_s),
    sizeof(struct htmlescape_s),
    &htmlescape_cmp) ;
  return p ? p->code : s ;
}
