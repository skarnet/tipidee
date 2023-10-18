/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/types.h>

#include <tipidee/conf.h>

char const *tipidee_conf_get_errorfile (tipidee_conf const *conf, char const *docroot, unsigned int status)
{
  size_t docrootlen = strlen(docroot) ;
  char key[7 + docrootlen] ;
  if (status < 100 || status > 999) goto err ;
  key[0] = 'E' ; key[1] = ':' ;
  uint_fmt(key + 2, status) ;
  key[5] = ':' ;
  memcpy(key + 6, docroot, docrootlen + 1) ;
  return tipidee_conf_get_string(conf, key) ;

 err:
  errno = EINVAL ;
  return 0 ;
}
