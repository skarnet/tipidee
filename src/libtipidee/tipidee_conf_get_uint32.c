/* ISC license. */

#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/cdb.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

int tipidee_conf_get_uint32 (tipidee_conf const *conf, char const *key, uint32_t *value)
{
  cdb_data data ;
  if (!tipidee_conf_get(conf, key, &data)) return 0 ;
  if (data.len != 4) return (errno = EPROTO, 0) ;
  uint32_unpack_big(data.s, value) ;
  return 1 ;
}
