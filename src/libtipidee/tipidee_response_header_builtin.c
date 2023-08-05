/* ISC license. */

#include <string.h>
#include <stdlib.h>

#include <tipidee/config.h>
#include <tipidee/response.h>

static tipidee_response_header_builtin const tipidee_response_header_builtin_table_[] =
{
  { .key = "Accept-Ranges", .value = "none" },
  { .key = "Cache-Control", .value = "private" },
  { .key = "Content-Security-Policy", .value = "default-src 'self'; style-src 'self' 'unsafe-inline';" },
  { .key = "Referrer-Policy", .value = "no-referrer-when-downgrade" },
  { .key = "Server", .value = "tipidee/" TIPIDEE_VERSION },
  { .key = "Vary", .value = "Accept-Encoding" },
  { .key = "X-Content-Type-Options", .value = "nosniff" },
  { .key = "X-Frame-Options", .value = "DENY" },
  { .key = "X-XSS-Protection", .value = "1; mode=block" },
  { .key = 0, .value = 0 },
} ;

tipidee_response_header_builtin const *tipidee_response_header_builtin_table = tipidee_response_header_builtin_table_ ;

static int tipidee_response_header_builtin_cmp (void const *a, void const *b)
{
  return strcmp((char const *)a, ((tipidee_response_header_builtin const *)b)->key) ;
}

char const *tipidee_response_header_builtin_search (char const *key)
{
  tipidee_response_header_builtin const *p = bsearch(
    key,
    tipidee_response_header_builtin_table_,
    sizeof(tipidee_response_header_builtin_table_) / sizeof(tipidee_response_header_builtin) - 1,
    sizeof(tipidee_response_header_builtin),
    &tipidee_response_header_builtin_cmp) ;
  return p ? p->value : 0 ;
}

