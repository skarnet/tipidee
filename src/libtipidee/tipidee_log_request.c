/* ISC license. */

#include <stddef.h>

#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

#include <tipidee/method.h>
#include <tipidee/headers.h>
#include <tipidee/log.h>

void tipidee_log_request (uint32_t v, tipidee_rql const *rql, tipidee_headers const *hdr, stralloc *sa)
{
  char const *a[16] = { PROG, ": info:" } ;
  size_t m = 2 ;
  size_t start = sa->len ;  /* assert: not zero */
  size_t refpos = 0, uapos = 0 ;
  if (!(v & TIPIDEE_LOG_REQUEST)) return ;
  if (!string_quotes(sa, rql->uri.path)) goto eerr ;
  if (v & TIPIDEE_LOG_REFERRER)
  {
    char const *x = tipidee_headers_search(hdr, "Referrer") ;
    if (x)
    {
      refpos = sa->len ;
      if (!string_quotes(sa, x) || !stralloc_0(sa)) goto err ;
    }
  }
  if (v & TIPIDEE_LOG_UA)
  {
    char const *x = tipidee_headers_search(hdr, "User-Agent") ;
    if (x)
    {
      uapos = sa->len ;
      if (!string_quotes(sa, x) || !stralloc_0(sa)) goto err ;
    }
  }
  if (v & TIPIDEE_LOG_HOSTASPREFIX)
  {
    a[m++] = " host " ;
    a[m++] = rql->uri.host ;
  }
  a[m++] = " request " ;
  a[m++] = tipidee_method_tostr(rql->m) ;
  if (!(v & TIPIDEE_LOG_HOSTASPREFIX))
  {
    a[m++] = " host " ;
    a[m++] = rql->uri.host ;
  }
  a[m++] = " path " ;
  a[m++] = sa->s + start ;
  if (rql->uri.query)
  {
    a[m++] = " query " ;
    a[m++] = rql->uri.query ;
  }
  a[m++] = " http 1." ;
  a[m++] = rql->http_minor ? "1" : "0" ;
  if (refpos)
  {
    a[m++] = " referrer " ;
    a[m++] = sa->s + refpos ;
  }
  if (uapos)
  {
    a[m++] = " user-agent " ;
    a[m++] = sa->s + uapos ;
  }
  strerr_warnv(a, m) ;
  sa->len = start ;
  return ;

 err:
  sa->len = start ;
 eerr:
  strerr_warnwu1sys("log request") ;
}
