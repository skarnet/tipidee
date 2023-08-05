/* ISC license. */

#include <errno.h>
#include <string.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

int tipidee_conf_get_redirection (tipidee_conf const *conf, char const *res, size_t docrootlen, tipidee_redirection *r)
{
  size_t reslen = strlen(res) ;
  size_t l = 2 + reslen ;
  char const *v = 0 ;
  char key[3 + reslen] ;
  key[0] = 'R' ; key[1] = ':' ;
  memcpy(key + 2, res, reslen) ;
  key[2 + reslen] = '/' ;
  errno = ENOENT ;
  while (!v)
  {
    if (errno != ENOENT) return -1 ;
    while (l > 2  + docrootlen && key[l] != '/') l-- ;
    if (l <= 2 + docrootlen) break ;
    key[l--] = 0 ;
    key[0] = 'r' ;
    v = tipidee_conf_get_string(conf, key) ;
  }
  if (!v) return 0 ;
  if (v[0] < '@' || v[0] > 'C') return (errno = EPROTO, -1) ;
  r->type = v[0] & ~'@' ;
  r->location = v+1 ;
  r->sub = res + l - 2 ;
  return 1 ;
}
