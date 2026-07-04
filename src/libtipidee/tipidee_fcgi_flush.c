/* ISC license. */

#include <skalibs/unix-timed.h>

#include <tipidee/fcgi.h>

int tipidee_fcgi_flush (tipidee_fcgi *fc, tain *stamp)
{
  return buffer_timed_flush(&fc->b, &fc->deadline, stamp) ;
}
