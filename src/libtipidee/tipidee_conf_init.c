/* ISC license. */

#include <skalibs/cdb.h>

#include <tipidee/conf.h>

int tipidee_conf_init (tipidee_conf *conf, char const *file)
{
  return cdb_init(&conf->c, file) ;
}
