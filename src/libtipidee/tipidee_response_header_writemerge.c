/* ISC license. */

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#include <skalibs/buffer.h>

#include <tipidee/headers.h>
#include <tipidee/response.h>

static int tipidee_response_header_cmp (void const *a, void const *b)
{
  return strcasecmp((char const *)a, ((tipidee_response_header const *)b)->key) ;
}

size_t tipidee_response_header_writemerge (buffer *b, tipidee_response_header const *rhdr, uint32_t rhdrn, tipidee_headers const *hdr, uint32_t options, tain const *stamp)
{
  char fmt[128] ;
  size_t m = buffer_putnoflush(b, fmt, tipidee_response_header_date(fmt, 128, stamp)) ;
  if (options & 1) m += buffer_putsnoflush(b, "Connection: close\r\n") ;

  for (uint32_t i = 0 ; i < rhdrn ; i++)
  {
    if (!rhdr[i].value) continue ;
    if (rhdr[i].options & 1 && tipidee_headers_search(hdr, rhdr[i].key)) continue ;
    m += buffer_putsnoflush(b, rhdr[i].key) ;
    m += buffer_putnoflush(b, ": ", 2) ;
    m += buffer_putsnoflush(b, rhdr[i].value) ;
    m += buffer_putnoflush(b, "\r\n", 2) ;
  }

  for (uint32_t i = 0 ; i < hdr->n ; i++)
  {
    tipidee_response_header const *p ;
    char const *key = hdr->buf + hdr->list[i].left ;
    if (!strncasecmp(key, "X-CGI-", 6)) continue ;
    p = bsearch(key, rhdr, rhdrn, sizeof(tipidee_response_header), &tipidee_response_header_cmp) ;
    if (p && !(p->options & 1)) continue ;
    m += buffer_putsnoflush(b, key) ;
    m += buffer_putnoflush(b, ": ", 2) ;
    m += buffer_putsnoflush(b, hdr->buf + hdr->list[i].right) ;
    m += buffer_putnoflush(b, "\r\n", 2) ;
  }

  return m ;
}
