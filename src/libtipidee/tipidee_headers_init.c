/* ISC license. */

#include <stdint.h>
#include <strings.h>

#include <skalibs/avltreen.h>

#include <tipidee/headers.h>

static void *tipidee_headers_dtok (uint32_t d, void *data)
{
  tipidee_headers *hdr = data ;
  return hdr->buf + hdr->list[d].left ;
}

static int tipidee_headers_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  return strcasecmp(a, b) ;
}

void tipidee_headers_init (tipidee_headers *hdr, char *buf, size_t max)
{
  hdr->buf = buf ;
  hdr->max = max ;
  hdr->len = 0 ;
  hdr->n = 0 ;
  avltreen_init(&hdr->map, hdr->map_storage, hdr->map_freelist, TIPIDEE_HEADERS_MAX, &tipidee_headers_dtok, &tipidee_headers_cmp, hdr) ;
}
