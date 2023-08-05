/* ISC license. */

#include <errno.h>

#include <skalibs/cdb.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

char const *tipidee_conf_get_string (tipidee_conf const *conf, char const *key)
{
  cdb_data data ;
  if (!tipidee_conf_get(conf, key, &data)) return 0 ;
  if (data.s[data.len-1]) { errno = EPROTO ; return 0 ; }
  return data.s ;
}
