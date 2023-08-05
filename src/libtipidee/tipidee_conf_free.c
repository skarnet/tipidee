/* ISC license. */

#include <skalibs/cdb.h>

#include <tipidee/conf.h>

void tipidee_conf_free (tipidee_conf *conf)
{
  cdb_free(&conf->c) ;
}
