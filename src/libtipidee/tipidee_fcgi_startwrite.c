/* ISC license. */

#include <skalibs/buffer.h>

#include <tipidee/fcgi.h>

void tipidee_fcgi_startwrite (tipidee_fcgi *fc, int fd, pid_t pid, tain const *deadline)
{
  buffer_init(&fc->b, &buffer_write, fd, fc->buf, 4096) ;
  fc->deadline = *deadline ;
  fc->id = pid & 0xffffu ; fc->id += !fc->id ;
}
