/* ISC license. */

#include <stdint.h>

#include <skalibs/avltreen.h>

#include <tipidee/headers.h>

char const *tipidee_headers_search (tipidee_headers const *hdr, char const *key)
{
  uint32_t i ;
  return avltreen_search(&hdr->map, key, &i) ? hdr->buf + hdr->list[i].right : 0 ;
}
