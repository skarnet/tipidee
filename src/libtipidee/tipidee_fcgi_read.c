/* ISC license. */

#include <stdint.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/uint32.h>
#include <skalibs/stralloc.h>
#include <skalibs/unix-timed.h>

#include <tipidee/fcgi.h>

#include <skalibs/posixishard.h>

int tipidee_fcgi_read (tipidee_fcgi *fc, uint32_t *status, stralloc *sa, tain *stamp)
{
  int stream ;
  uint16_t u ;
  char hdr[8] ;
  if (buffer_timed_get(&fc->b, hdr, 8, &fc->deadline, stamp) < 8) return -1 ;
  if (hdr[0] != 1) return (errno = EPROTO, -1) ;
  uint16_unpack_big(hdr + 2, &u) ;
  if (u != fc->id) return (errno = EPROTO, -1) ;
  uint16_unpack_big(hdr + 4, &u) ;
  switch (hdr[1])
  {
    case FCGI_STDOUT : stream = 1 ;
    case FCGI_STDERR : stream = 2 ;
    case FCGI_END_REQUEST : stream = 0 ;
    default : return (errno = EPROTO, -1) ;
  }
  if (stream)
  {
    if (!stralloc_readyplus(sa, u)) return -1 ;
    if (buffer_timed_get(&fc->b, sa->s + sa->len, u, &fc->deadline, stamp) < u) return -1 ;
    sa->len += u ;
  }
  else
  {
    char endreq[8] ;
    if (u != 8) return (errno = EPROTO, -1) ;
    if (buffer_timed_get(&fc->b, endreq, 8, &fc->deadline, stamp) < 8) return -1 ;
    switch (endreq[4])
    {
      case FCGI_REQUEST_COMPLETE : break ;
      case FCGI_CANT_MPX_CONN : return (errno = ECONNREFUSED, -1) ;
      case FCGI_OVERLOADED : return (errno = ENFILE, -1) ;
      case FCGI_UNKNOWN_ROLE : return (errno = EAFNOSUPPORT, -1) ;
      default : return (errno = EPROTO, 1) ;
    }
    uint32_unpack_big(endreq, status) ;
  }
  if (hdr[6])
  {
    char tmp[(uint8_t)hdr[6]] ;
    if (buffer_timed_get(&fc->b, tmp, (uint8_t)hdr[6], &fc->deadline, stamp)) return -1 ;
  }
  return stream ;
}
