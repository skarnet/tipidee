/* ISC license. */

#include <string.h>

#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/unix-timed.h>

#include <tipidee/method.h>
#include <tipidee/response.h>
#include "tipideed-internal.h"

int respond_trace (char const *buf, tipidee_rql const *rql, tipidee_headers const *hdr)
{
  tain deadline ;
  size_t cl = 0 ;
  char fmt[SIZE_FMT] ;
  tipidee_response_status_line(buffer_1, rql, "200 OK") ;
  tipidee_response_header_common_put_g(buffer_1, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Type: message/http\r\nContent-Length: ") ;
  cl += strlen(tipidee_method_tostr(rql->m)) + 1;
  if (rql->uri.host) cl += 7 + rql->uri.https + strlen(rql->uri.host) ;
  cl += strlen(rql->uri.path) + (rql->uri.query ? 1 + strlen(rql->uri.query) : 0) ;
  cl += 6 + uint_fmt(0, rql->http_major) + 1 + uint_fmt(0, rql->http_minor) + 2 ;
  for (size_t i = 0 ; i < hdr->n ; i++)
    cl += strlen(buf + hdr->list[i].left) + 2 + strlen(buf + hdr->list[i].right) + 2 ;
  cl += 2 ;
  buffer_putnoflush(buffer_1, fmt, size_fmt(fmt, cl)) ;
  buffer_putsnoflush(buffer_1, "\r\n\r\n") ;

  buffer_putsnoflush(buffer_1, tipidee_method_tostr(rql->m)) ;
  buffer_putnoflush(buffer_1, " ", 1) ;
  if (rql->uri.host)
  {
    buffer_putsnoflush(buffer_1, rql->uri.https ? "https://" : "http://") ;
    buffer_putsnoflush(buffer_1, rql->uri.host) ;
  }
  buffer_putsnoflush(buffer_1, rql->uri.path) ;
  if (rql->uri.query)
  {
    buffer_putnoflush(buffer_1, "?", 1) ;
    buffer_putsnoflush(buffer_1, rql->uri.query) ;
  }
  buffer_putsnoflush(buffer_1, " HTTP/") ;
  buffer_putnoflush(buffer_1, fmt, uint_fmt(fmt, rql->http_major)) ;
  buffer_putnoflush(buffer_1, ".", 1) ;
  buffer_putnoflush(buffer_1, fmt, uint_fmt(fmt, rql->http_minor)) ;
  buffer_putsnoflush(buffer_1, "\r\n") ;
  for (size_t i = 0 ; i < hdr->n ; i++)
  {
    size_t len = strlen(buf + hdr->list[i].left) ;
    tain_add_g(&deadline, &g.writetto) ;
    if (buffer_timed_put_g(buffer_1, buf + hdr->list[i].left, len, &deadline) < len) goto err ;
    if (buffer_timed_put_g(buffer_1, ": ", 2, &deadline) < 2) goto err ;
    len = strlen(buf + hdr->list[i].right) ;
    if (buffer_timed_put_g(buffer_1, buf + hdr->list[i].right, len, &deadline) < len) goto err ;
    if (buffer_timed_put_g(buffer_1, "\r\n", 2, &deadline) < 2) goto err ;
  }
  if (buffer_timed_put_g(buffer_1, "\r\n", 2, &deadline) < 2
   || !buffer_timed_flush_g(buffer_1, &deadline)) goto err ;
  return 0 ;

 err:
  strerr_diefu1sys(111, "write to stdout") ;
}
