/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/lolstdio.h>

#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

static int get_redir (tipidee_conf const *conf, size_t minl, char *key, size_t l, char const *path, tipidee_redirection *r)
{
  char const *v ;
  key[0] = 'R' ;
  key[l] = '/' ;
  for (;;)
  {
    while (l > minl && key[l] != '/') l-- ;
    if (l <= minl) return 0 ;
    key[l--] = 0 ;
    v = tipidee_conf_get_string(conf, key) ;
    if (v) break ;
    if (errno != ENOENT) return -1 ;
    key[0] = 'r' ;
  }
  if (v[0] < '@' || v[0] > 'C') return (errno = EPROTO, -1) ;
  r->type = v[0] & ~'@' ;
  r->location = v+1 ;
  r->sub = path + (l - minl + 1) ;
  LOLDEBUG("get_redir: found redirection of type %c to %s with sub %s", v[0], r->location, r->sub) ;
  return 1 ;
}

int tipidee_conf_get_redirection (tipidee_conf const *conf, char const *docroot, size_t docrootlen, char const *path, tipidee_redirection *r)
{
  size_t pathlen = strlen(path) ;
  char key[docrootlen + pathlen + 3] ;
  key[1] = ':' ;
  memcpy(key + 2, docroot, docrootlen) ;
  memcpy(key + 2 + docrootlen, path, pathlen + 1) ;
  {
    int e = get_redir(conf, 2 + docrootlen, key, 2 + docrootlen + pathlen, path, r) ;
    if (e) return e ;
  }
  {
    char *p = memchr(docroot, ':', docrootlen) ;
    if (!p) return 0 ;
    docrootlen = p - docroot ;
  }
  memcpy(key + 2, docroot, docrootlen) ;
  memcpy(key + 2 + docrootlen, path, pathlen + 1) ;
  return get_redir(conf, 2 + docrootlen, key, 2 + docrootlen + pathlen, path, r) ;
}
