/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <strings.h>

#include <skalibs/types.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/unix-timed.h>
// #include <skalibs/lolstdio.h>

#include <tipidee/method.h>
#include <tipidee/uri.h>
#include <tipidee/rql.h>

static inline uint8_t tokenize_cclass (char c)
{
  switch (c)
  {
    case '\0' : return 0 ;
    case ' ' :
    case '\t' : return 1 ;
    default : return 2 ;
  }
}

static inline int rql_tokenize (char *s, size_t *tab)
{
  uint8_t const table[2][3] =
  {
    { 0x02, 0x00, 0x11 },
    { 0x02, 0x20, 0x01 }
  } ;
  size_t i = 0 ;
  unsigned int tokens = 0 ;
  uint8_t state = 0 ;
  for (; state < 2 ; i++)
  {
    uint8_t c = table[state][tokenize_cclass(s[i])] ;
    state = c & 3 ;
    if (c & 0x10)
    {
      if (tokens >= 3) goto err ;
      tab[tokens++] = i ;
    }
    if (c & 0x20) s[i] = 0 ;
  }
  return 1 ;
 err:
  return 0 ;
}

static inline int get_version (char const *in, tipidee_rql *rql)
{
  size_t l ;
  if (strncmp(in, "HTTP/", 5)) return 0 ;
  in += 5 ;
  l = uint_scan(in, &rql->http_major) ;
  if (!l) return 0 ;
  in += l ;
  if (*in++ != '.') return 0 ;
  return !!uint0_scan(in, &rql->http_minor) ;
}

int tipidee_rql_read (buffer *b, char *buf, size_t max, size_t *w, tipidee_rql *rql, tain const *deadline, tain *stamp)
{
  size_t pos[3] = { 0 } ;
  if (timed_getlnmax(b, buf, max, &pos[0], '\n', deadline, stamp) <= 0) return -1 ;
  buf[--pos[0]] = 0 ;
  if (buf[pos[0] - 1] == '\r') buf[--pos[0]] = 0 ;
//  LOLDEBUG("tipidee_rql_read: timed_getlnmax: len is %zu, line is %s", pos[0], buf) ;
  if (!rql_tokenize(buf, pos)) return 400 ;
//  LOLDEBUG("tipidee_rql_read: method: %s, version: %s, uri to parse: %s", buf + pos[0], buf + pos[2], buf + pos[1]) ;
  rql->m = tipidee_method_tonum(buf + pos[0]) ;
  if (rql->m == TIPIDEE_METHOD_UNKNOWN) return 400 ;
  if (!get_version(buf + pos[2], rql)) return 400 ;
  if (rql->m != TIPIDEE_METHOD_OPTIONS || strcmp(buf + pos[1], "*"))
  {
    size_t l = tipidee_uri_parse(buf, max, buf + pos[1], &rql->uri) ;
    if (!l) return 400 ;
    *w = l ;
  }
  return 0 ;
}
