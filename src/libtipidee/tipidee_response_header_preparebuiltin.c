/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <tipidee/response.h>

int tipidee_response_header_preparebuiltin (tipidee_response_header *tab, uint32_t n, char const *s, size_t len)
{
  size_t pos = 0 ;
  for (uint32_t i = 0 ; i < n ; i++)
  {
    char const *next ;
    tab[i].key = s + pos ;
    next = memchr(s + pos, 0, len - pos) ;
    if (!next) return 0 ;
    pos = next - s ;
    if (pos++ >= len) return 0 ;
    tab[i].options = (uint8_t)s[pos] ;
    if (pos++ >= len) return 0 ;
    tab[i].value = s + pos ;
    next = memchr(s + pos, 0, len - pos) ;
    if (!next) return 0 ;
    pos = next - s ;
    if (pos++ >= len) return 0 ;
  }
  return pos == len ;
}
