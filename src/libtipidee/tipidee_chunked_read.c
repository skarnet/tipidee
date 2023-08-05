/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/types.h>
#include <skalibs/stralloc.h>
#include <skalibs/unix-timed.h>

#include <tipidee/body.h>

#include <skalibs/posixishard.h>

int tipidee_chunked_read (buffer *b, stralloc *sa, size_t maxlen, tain const *deadline, tain *stamp)
{
  char line[512] ;
  for (;;)
  {
    size_t chunklen, pos, w = 0 ;
    ssize_t r = timed_getlnmax(b, line, 512, &w, '\n', deadline, stamp) ;
    if (r < 0) return 0 ;
    if (!r) return (errno = EPIPE, 0) ;
    pos = size_scan(line, &chunklen) ;
    if (!pos) return (errno = EPROTO, 0) ;
    if (!memchr("\r\n; \t", line[pos], 5)) return (errno = EPROTO, 0) ;
    if (!chunklen) break ;
    if (sa->len + chunklen > maxlen) return (errno = EMSGSIZE, 0) ;
    if (!stralloc_readyplus(sa, chunklen)) return 0 ;
    if (buffer_timed_get(b, sa->s + sa->len, chunklen, deadline, stamp) < chunklen) return 0 ;
    sa->len += chunklen ;
  }
  for (;;)
  {
    size_t w = 0 ;
    ssize_t r = timed_getlnmax(b, line, 512, &w, '\n', deadline, stamp) ;
    if (r < 0) return 0 ;
    if (!r) return (errno = EPIPE, 0) ;
    if (w == 1 || (w == 2 && line[0] == '\r')) break ;
  }
  return 1 ;
}
