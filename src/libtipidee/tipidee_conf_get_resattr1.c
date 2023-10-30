/* ISC license. */

#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <skalibs/uint32.h>
#include <skalibs/cdb.h>

#include <tipidee/resattr.h>
#include <tipidee/conf.h>

#include <skalibs/posixishard.h>

int tipidee_conf_get_resattr1 (tipidee_conf const *conf, char const *key, tipidee_resattr *ra)
{
  tipidee_resattr atom = TIPIDEE_RESATTR_ZERO ;
  cdb_data data ;
  if (!tipidee_conf_get(conf, key, &data)) return errno == ENOENT ? 0 : -1 ;
  if (data.len < 9 || data.s[data.len - 1]) return (errno = EPROTO, -1) ;
  uint32_unpack_big(data.s, &atom.flags) ;
  uint32_unpack_big(data.s + 4, &atom.mask) ;
  data.s += 8 ; data.len -= 8 ;
  if (*data.s)
  {
    size_t len = strlen(data.s) + 1 ;
    if (len > data.len) return (errno = EPROTO, -1) ;
    atom.content_type = data.s ;
    data.s += len ;
    data.len -= len ;
  }
  else data.len-- ;
  if (data.len) return (errno = EPROTO, -1) ;
  *ra = atom ;
  return 1 ;
}
