/* ISC license. */

#include <tipidee/fcgi.h>

int tipidee_fcgi_beginrequest (tipidee_fcgi *fc, tain *stamp)
{
  char body[8] = { 0, FCGI_RESPONDER, FCGI_KEEP_CONN, 0, 0, 0, 0, 0 } ;
  return tipidee_fcgi_put(fc, FCGI_BEGIN_REQUEST, body, 8, stamp) == 8 ;
}
