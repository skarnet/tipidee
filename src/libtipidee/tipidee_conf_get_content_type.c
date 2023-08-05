/* ISC license. */

#include <errno.h>
#include <string.h>

#include <tipidee/conf.h>

char const *tipidee_conf_get_content_type (tipidee_conf const *conf, char const *res)
{
  char const *ext = strrchr(res, '.') ;
  if (ext && !strchr(ext, '/'))
  {
    char const *value = 0 ;
    size_t extlen = strlen(ext+1) ;
    char key[3 + extlen] ;
    key[0] = 'T' ; key[1] = ':' ;
    memcpy(key + 2, ext + 1, extlen + 1) ;
    value = tipidee_conf_get_string(conf, key) ;
    if (value || errno != ENOENT) return value ;
  }
  return "application/octet-stream" ;
}
