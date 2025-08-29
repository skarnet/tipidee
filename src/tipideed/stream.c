/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASSPLICE

#include <skalibs/nonposix.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>

#include <skalibs/uint64.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-timed.h>

#include "tipideed-internal.h"

struct fixed_info_s
{
  int fd ;
  uint64_t n ;
  uint32_t realtime : 1 ;
} ;

static int fixed_getfd (void *b)
{
  struct fixed_info_s *si = b ;
  return si->fd ;
}

static ssize_t fixed_get (void *b)
{
  struct fixed_info_s *si = b ;
  while (si->n)
  {
    ssize_t r = sanitize_read(splice(si->fd, 0, 1, 0, si->n > SSIZE_MAX ? SSIZE_MAX : si->n, SPLICE_F_NONBLOCK | (!si->realtime && si->n > 65536 ? SPLICE_F_MORE : 0))) ;
    if (r == -1) break ;
    if (!r) return 0 ;
    si->n -= r ;
  }
  return si->n ? -1 : 1 ;
}

void cork (int fd)
{
  static int const val = 1 ;
  if (setsockopt(fd, SOL_TCP, TCP_CORK, &val, sizeof(int)) == -1 && g.logv)
    strerr_warnwu1sys("uncork stdout") ;
}

void uncork (int fd)
{
  static int const val = 0 ;
  if (setsockopt(fd, SOL_TCP, TCP_CORK, &val, sizeof(int)) == -1 && g.logv)
    strerr_warnwu1sys("uncork stdout") ;
}

void stream_fixed (int fd, uint64_t n, char const *fn, uint32_t flags)
{
  tain deadline ;
  struct fixed_info_s si = { .fd = fd, .n = n, .realtime = !!(flags & TIPIDEE_RA_FLAG_REALTIME) } ;
  tain_add_g(&deadline, tain_less(&g.writetto, &g.cgitto) ? &g.cgitto : &g.writetto) ;
  if (timed_get_g(&si, &fixed_getfd, &fixed_get, &deadline) < 0)
    strerr_diefu3sys(111, "splice from ", fn, " to stdout") ;
}

void stream_infinite (int fd, char const *fn, uint32_t flags)
{
  ssize_t r ;

  if (ndelay_off(fd) == -1 || ndelay_off(1) == -1) strerr_diefu1sys(111, "set fds blocking") ;

   /* only works when fd is a pipe, which is the case for cgi; fcgi/scgi will need refactoring */
   /* XXX: ignores timeouts, but this is just TOO GOOD to pass up */
   /* no really, it is just optimal in 99.9% of cases, it is WORTH IT */

    while ((r = splice(fd, 0, 1, 0, 65536, flags & TIPIDEE_RA_FLAG_REALTIME ? 0 : SPLICE_F_MORE)) > 0) ;

   /* You WISH you had written that line of code */

  if (r == -1) strerr_diefu3sys(111, "splice from ", fn, " to stdout") ;
  if (ndelay_on(1) == -1) strerr_diefu1sys(111, "set stdout nonblocking again") ;
}

#else

#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>

#include <skalibs/uint64.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/siovec.h>
#include <skalibs/socket.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include "tipideed-internal.h"

void cork (int fd)
{
  socket_tcpdelay(fd) ;
}

void uncork (int fd)
{
  socket_tcpnodelay(fd) ;
}

void stream_fixed (int fd, uint64_t n, char const *fn, uint32_t flags)
{
  tain deadline ;
  struct iovec v[2] ;
  size_t r ;
  while (n)
  {
    buffer_wpeek(buffer_1, v) ;
    siovec_trunc(v, 2, n) ;
    tain_add_g(&deadline, &g.cgitto) ;
    r = timed_readv_g(fd, v, 2, &deadline) ;
    if (r == -1) strerr_diefu2sys(111, "read from resource ", fn) ;
    if (!r) strerr_dief3x(111, "resource ", fn, " provided less content than advertised in Content-Length") ;
    buffer_wseek(buffer_1, r) ;
    n -= r ;
    tain_add_g(&deadline, &g.writetto) ;
    if (!buffer_timed_flush_g(buffer_1, &deadline))
      strerr_diefu1sys(111, "write to stdout") ;
  }
  (void)flags ;
}

void stream_infinite (int fd, char const *fn, uint32_t flags)
{
  tain deadline ;
  struct iovec v[2] ;
  size_t r ;
  for (;;)
  {
    buffer_wpeek(buffer_1, v) ;
    tain_add_g(&deadline, &g.cgitto) ;
    r = timed_readv_g(fd, v, 2, &deadline) ;
    if (r == -1) strerr_diefu2sys(111, "read from resource ", fn) ;
    if (!r) break ;
    buffer_wseek(buffer_1, r) ;
    tain_add_g(&deadline, &g.writetto) ;
    if (!buffer_timed_flush_g(buffer_1, &deadline))
      strerr_diefu1sys(111, "write to stdout") ;
  }
  (void)flags ;
}

#endif

#include <skalibs/types.h>
#include <skalibs/buffer.h>

void stream_autochunk (buffer *b, char const *fn)
{
  tain deadline ;
  struct iovec v[2] ;
  size_t len, w ;
  char fmt[SIZE_XFMT] ;

  for (;;)
  {
    ssize_t r = buffer_len(b) ;
    if (!r)
    {
      tain_add_g(&deadline, &g.cgitto) ;
      r = buffer_timed_fill_g(b, &deadline) ;
      if (r == -1) strerr_diefu2sys(111, "read from resource ", fn) ;
      if (!r) break ;
    }

    tain_add_g(&deadline, &g.writetto) ;
    len = size_xfmt(fmt, r) ;
    if (buffer_timed_put_g(buffer_1, fmt, len, &deadline) < len
     || buffer_timed_put_g(buffer_1, "\r\n", 2, &deadline) < 2) strerr_diefu1sys(111, "write to stdout") ;
    buffer_rpeek(b, v) ;
    w = buffer_timed_putv_g(buffer_1, v, 2, &deadline) ;
    buffer_rseek(b, w) ;
    if (w < r
     || buffer_timed_put_g(buffer_1, "\r\n", 2, &deadline) < 2
     || !buffer_timed_flush_g(buffer_1, &deadline)) strerr_diefu1sys(111, "write to stdout") ;
  }

  tain_add_g(&deadline, &g.writetto) ;
  if (buffer_timed_put_g(buffer_1, "0\r\n\r\n", 5, &deadline) < 5
   || !buffer_timed_flush_g(buffer_1, &deadline)) strerr_diefu1sys(111, "write to stdout") ;
}
