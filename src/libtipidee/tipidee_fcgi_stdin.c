/* ISC license. */

#include <tipidee/fcgi.h>

int tipidee_fcgi_stdin (tipidee_fcgi *fc, char const *s, size_t len, tain *stamp)
{
  return tipidee_fcgi_put(fc, FCGI_STDIN, s, len, stamp) == len ;
}
