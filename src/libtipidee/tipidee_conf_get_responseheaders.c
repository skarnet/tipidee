/* ISC license. */

#include <stddef.h>
#include <errno.h>
#include <string.h>

#include <skalibs/uint32.h>
#include <skalibs/cdb.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

char const *tipidee_conf_get_responseheaders (tipidee_conf const *conf, char const *key, uint32_t *len, uint32_t *n)
{
  cdb_data data ;
  if (!tipidee_conf_get(conf, key, &data)) return NULL ;
  if (data.len < 4) return (errno = EPROTO, NULL) ;
  uint32_unpack_big(data.s + data.len - 4, n) ;
  *len = data.len - 4 ;
  return data.s ;
}
