/* ISC license. */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <skalibs/uint16.h>
#include <skalibs/cdb.h>

#include <tipidee/redirection.h>
#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

static int get_redir (tipidee_conf const *conf, size_t minl, char *key, size_t l, char const *path, tipidee_redirection *rd)
{
  cdb_data data ;
  int r = 0 ;
  key[0] = 'R' ;
  key[l] = '/' ;
  errno = ENOENT ;
  while (!r)
  {
    if (errno != ENOENT) return -1 ;
    while (l >= minl && key[l] != '/') l-- ;
    if (l < minl) return 0 ;
    key[l--] = 0 ;
    r = tipidee_conf_get(conf, key, &data) ;
    key[0] = 'r' ;
  }
  if (data.len < 5) return (errno = EILSEQ, -1) ;
  rd->sub = path + (l - minl + 1) ;
  rd->type = *data.s++ ; data.len-- ;
  if (!rd->type)
    return data.len ? (errno = EILSEQ, -1) : 1 ;
  rd->flags = *data.s++ ; data.len-- ;
  if (rd->type >= 2 && rd->flags & TIPIDEE_REDIR_ISINET)
  {
    if (data.len != (rd->flags & TIPIDEE_REDIR_ISV6 ? 18 : 6))
      return (errno = EILSEQ, -1) ;
    uint16_unpack_big(data.s, &rd->port) ;
    data.s += 2 ; data.len -= 2 ;
  }
  else if (data.s[data.len - 1])
    return (errno = EILSEQ, -1) ;
  rd->addr = data.s ;
  return 1 ;
}

int tipidee_conf_get_redirection (tipidee_conf const *conf, char const *docroot, size_t docrootlen, char const *path, tipidee_redirection *rd)
{
  size_t pathlen = strlen(path) ;
  char key[docrootlen + pathlen + 3] ;
  key[1] = ':' ;
  memcpy(key + 2, docroot, docrootlen) ;
  memcpy(key + 2 + docrootlen, path, pathlen + 1) ;
  {
    int e = get_redir(conf, 2 + docrootlen, key, 2 + docrootlen + pathlen, path, rd) ;
    if (e) return e ;
  }
  {
    char *p = memchr(docroot, ':', docrootlen) ;
    if (!p) return 0 ;
    docrootlen = p - docroot ;
  }
  memcpy(key + 2, docroot, docrootlen) ;
  memcpy(key + 2 + docrootlen, path, pathlen + 1) ;
  return get_redir(conf, 2 + docrootlen, key, 2 + docrootlen + pathlen, path, rd) ;
}
