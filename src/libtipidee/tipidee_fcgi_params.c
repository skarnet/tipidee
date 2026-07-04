/* ISC license. */

#include <tipidee/fcgi.h>

int tipidee_fcgi_params (tipidee_fcgi *fc, char const *s, size_t len, tain *stamp)
{
  return tipidee_fcgi_put(fc, FCGI_PARAMS, s, len, stamp) == len ;
}
