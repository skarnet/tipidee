/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASSENDFILE

#include <sys/types.h>
#include <sys/sendfile.h>
#include <limits.h>

#include <skalibs/uint64.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include "tipideed-internal.h"

void init_splice_pipe (void)
{
}

struct sendfile_s
{
  int fd ;
  off_t pos ;  
  uint64_t n ;
} ;

static int s_getfd (void *p)
{
  (void)p ;
  return 1 ;
}

static int s_isnonempty (void *p)
{
  struct sendfile_s *sf = p ;
  return !!sf->n ;
}

static int s_flush (void *p)
{
  struct sendfile_s *sf = p ;
  while (sf->n)
  {
    ssize_t r = sendfile(1, sf->fd, &sf->pos, sf->n > SSIZE_MAX ? SSIZE_MAX : sf->n) ;
    if (r == -1) return 0 ;
    sf->n -= r ;
  }
  return 1 ;
}

void send_file_range (int fd, uint64_t offset, uint64_t n, char const *fn)
{
  struct sendfile_s sf = { .pos = offset, .n = n, .fd = fd } ;
  tain deadline ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu2sys(111, "write", " to stdout") ;
  if (!timed_flush_g(&sf, &s_getfd, &s_isnonempty, &s_flush, &deadline))
    strerr_diefu3sys(111, "sendfile ", fn, " to stdout") ;
}

#else
#ifdef SKALIBS_HASSPLICE

#include <skalibs/nonposix.h>

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include <skalibs/uint64.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-timed.h>

#include "tipideed-internal.h"

void init_splice_pipe (void)
{
  if (pipenbcoe(g.p) == -1)
    strerr_diefu1sys(111, "pipe2") ;
}

struct spliceinfo_s
{
  ssize_t n ;
  uint32_t last : 1 ;
} ;

static int getfd (void *b)
{
  (void)b ;
  return 1 ;
}

static int isnonempty (void *b)
{
  struct spliceinfo_s *si = b ;
  return !!si->n ;
}

static int flush (void *b)
{
  struct spliceinfo_s *si = b ;
  while (si->n)
  {
    ssize_t r = splice(g.p[0], 0, 1, 0, si->n, SPLICE_F_NONBLOCK | (si->last ? 0 : SPLICE_F_MORE)) ;
    if (r == -1) return 0 ;
    if (!r) return 1 ;
    si->n -= r ;
  }
  return 1 ;
}

void send_file_range (int fd, uint64_t offset, uint64_t n, char const *fn)
{
  tain deadline ;
  struct spliceinfo_s si = { .last = 0 } ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu2sys(111, "write", " to stdout") ;
  while (n)
  {
    si.n = splice(fd, &offset, g.p[1], 0, n, 0) ;
    if (si.n == -1) strerr_diefu2sys(111, "read from ", fn) ;
    else if (!si.n) strerr_diefu3x(111, "serve ", fn, ": file was truncated") ;
    else if (si.n > n)
    {
      si.n = n ;
      strerr_warnw2x("serving elongated file: ", fn) ;
    }
    n -= si.n ;
    if (!n) si.last = 1 ;
    tain_add_g(&deadline, &g.writetto) ;
    if (!timed_flush_g(&si, &getfd, &isnonempty, &flush, &deadline))
      strerr_diefu2sys(111, "splice", " to stdout") ;
  }
}

#else

#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>

#include <skalibs/uint64.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include "tipideed-internal.h"

void init_splice_pipe (void)
{
}

void send_file_range (int fd, uint64_t offset, uint64_t n, char const *fn)
{
  tain deadline ;
  struct iovec v[2] ;
  ssize_t r ;
  if (!n) goto flushit ;  /* I know, I know, but do-while SUCKS */
  if (offset && lseek(fd, offset, SEEK_SET) == -1)
    strerr_diefu2sys(111, "lseek on ", fn) ;
 fillit:
  buffer_wpeek(buffer_1, v) ;
  r = allreadv(fd, v, 2) ;
  if (r == -1) strerr_diefu2sys(111, "read from ", fn) ;
  if (!r) strerr_diefu3x(111, "serve ", fn, ": file was truncated") ;
  if (r > n)
  {
    r = n ;
    strerr_warnw2x("serving elongated file: ", fn) ;
  }
  buffer_wseek(buffer_1, r) ;
  n -= r ;
 flushit:
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  if (n) goto fillit ;
}

#endif
#endif
