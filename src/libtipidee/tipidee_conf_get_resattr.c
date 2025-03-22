/* ISC license. */

#include <string.h>

#include <skalibs/bytestr.h>

#include <tipidee/conf.h>
#include <tipidee/resattr.h>

#define FULLMASK ((1 << TIPIDEE_RA_BITS) - 1)

int tipidee_conf_get_resattr (tipidee_conf const *conf, char const *res, tipidee_resattr *ra)
{
  tipidee_resattr rra = *ra ;
  size_t len = strlen(res) + 2 ;
  char key[len + 1] ;
  key[0] = 'A' ; key[1] = ':' ;
  memcpy(key + 2, res, len - 2) ;
  key[len] = '/' ;
  while (len > 2 && (rra.mask & FULLMASK) != FULLMASK)
  {
    tipidee_resattr atom ;
    int r ;
    while (len > 2 && key[len] != '/') len-- ;
    if (len <= 2) break ;
    key[len--] = 0 ;
    r = tipidee_conf_get_resattr1(conf, key, &atom) ;
    if (r == -1) return 0 ;
    if (r)
    {
      rra.flags = (~rra.mask & atom.mask & atom.flags) | ((rra.mask | ~atom.mask) & rra.flags) ;  /* it's obvious, right */
      rra.mask |= atom.mask ;
      if (!rra.content_type) rra.content_type = atom.content_type ;
    }
    key[0] = 'a' ;
  }

  if (!(rra.mask & TIPIDEE_RA_FLAG_NPH))
  {
    char const *nphprefix ;
    char *p = strchr(key+2, '/') ;
    if (p) *p = 0 ;
    key[0] = 'N' ;
    nphprefix = tipidee_conf_get_string(conf, key) ;
    if (nphprefix)
    {
      char const *base = strrchr(res, '/') ;
      if (base && str_start(base + 1, nphprefix)) rra.flags |= 1 << TIPIDEE_RA_FLAG_NPH ;
      else rra.flags &= ~(1 << TIPIDEE_RA_FLAG_NPH) ;
      rra.mask |= 1 << TIPIDEE_RA_FLAG_NPH ;
    }
  }

  if (!(rra.flags & TIPIDEE_RA_FLAG_CGI) && !rra.content_type)
  {
    rra.content_type = tipidee_conf_get_content_type(conf, res) ;
    if (!rra.content_type) return 0 ;
  }

  *ra = rra ;
  return 1 ;
}
