/* ISC license. */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <skalibs/cdb.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

uint32_t tipidee_conf_get_argv (tipidee_conf const *conf, char const *key, char const **argv, uint32_t max, size_t *maxlen)
{
  cdb_data data ;
  size_t curlen = 0, pos = 0 ;
  uint32_t n = 0 ;
  if (!tipidee_conf_get(conf, key, &data)) return 0 ;
  if (data.s[data.len-1]) return (errno = EPROTO, 0) ;
  while (pos < data.len)
  {
    size_t len ;
    if (n >= max) return (errno = E2BIG, 0) ;
    argv[n++] = data.s + pos ;
    len = strlen(data.s + pos) ;
    if (len > curlen) curlen = len ;
    pos += len + 1 ;
  }
  if (n >= max) return (errno = E2BIG, 0) ;
  argv[n++] = 0 ;
  if (maxlen) *maxlen = curlen ;
  return n ;
}
