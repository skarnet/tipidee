/* ISC license. */

#include <sys/uio.h>
#include <stdint.h>
#include <errno.h>

#include <skalibs/uint16.h>
#include <skalibs/buffer.h>
#include <skalibs/siovec.h>
#include <skalibs/unix-timed.h>

#include <tipidee/fcgi.h>

static uint16_t putvit (tipidee_fcgi *fc, uint8_t rectype, struct iovec const *v, unsigned int n, uint16_t len, tain *stamp)
{
  char hdr[8] = { 1, (char)rectype, fc->id >> 8, fc->id & 0xff, 0, 0, 0, 0 } ;
  uint16_t w ;
  uint16_pack_big(hdr + 4, len) ;
  w = buffer_timed_put(&fc->b, hdr, 8, &fc->deadline, stamp) ;
  if (w < 8) return 0 ;
  return buffer_timed_putv(&fc->b, v, n, &fc->deadline, stamp) ;
}

size_t tipidee_fcgi_putv (tipidee_fcgi *fc, uint8_t rectype, struct iovec const *v, unsigned int n, tain *stamp)
{
  if (!n) return 0 ;
  size_t w = 0 ;
  unsigned int base = 0 ;
  for (unsigned int i = 0 ; i < n ; i++)
    if (v[i].iov_len > 0xffffu) return (errno = EMSGSIZE, 0) ;
  while (base < n)
  {
    size_t len = 0 ;
    unsigned int i = 0 ;
    uint16_t r ;
    for (; base + i < n ; i++)
    {
      if (len + v[base + i].iov_len > 0xffffu) break ;
      len += v[base + i].iov_len ;
    }
    r = putvit(fc, rectype, v + base, i, len, stamp) ;
    w += r ;
    if (r < len) break ;
    base += i ;
  }
  return w ;
}
