/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/cdb.h>
#include <skalibs/lolstdio.h>

#include <tipidee/conf.h>

int tipidee_conf_get (tipidee_conf const *conf, char const *key, cdb_data *data)
{
  size_t keylen = strlen(key) ;
  LOLDEBUG("tipidee_conf_get: looking up %s", key) ;
  if (keylen > TIPIDEE_CONF_KEY_MAXLEN) return (errno = EINVAL, 0) ;
  switch (cdb_find(&conf->c, data, key, keylen))
  {
    case -1 : return (errno = EILSEQ, 0) ;
    case 0 : return (errno = ENOENT, 0) ;
    default : return 1 ;
  }
}
