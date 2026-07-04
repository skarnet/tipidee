/* ISC license. */

#include <stdint.h>

#include <skalibs/uint16.h>
#include <skalibs/buffer.h>
#include <skalibs/unix-timed.h>

#include <tipidee/fcgi.h>

static uint16_t putit (tipidee_fcgi *fc, uint8_t rectype, char const *s, uint16_t len, tain *stamp)
{
  char hdr[8] = { 1, (char)rectype, fc->id >> 8, fc->id & 0xff, 0, 0, 0, 0 } ;
  uint16_t w ;
  uint16_pack_big(hdr + 4, len) ;
  w = buffer_timed_put(&fc->b, hdr, 8, &fc->deadline, stamp) ;
  if (w < 8) return 0 ;
  return buffer_timed_put(&fc->b, s, len, &fc->deadline, stamp) ;
}

size_t tipidee_fcgi_put (tipidee_fcgi *fc, uint8_t rectype, char const *s, size_t len, tain *stamp)
{
  size_t w = 0 ;
  while (len > 0xffffu)
  {
    uint16_t r = putit(fc, rectype, s, 0xffffu, stamp) ;
    w += r ;
    if (r < 0xffffu) return w ;
    s += 0xffff ;
    len -= 0xffff ;
  }
  w += putit(fc, rectype, s, len, stamp) ;
  return w ;
}
