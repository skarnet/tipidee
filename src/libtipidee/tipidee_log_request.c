/* ISC license. */

#include <stddef.h>

#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>

#include <tipidee/method.h>
#include <tipidee/log.h>

void tipidee_log_request (uint32_t v, tipidee_rql const *rql, char const *referrer, char const *ua, stralloc *sa)
{
  char const *a[14] = { "info: request " } ;
  size_t m = 1 ;
  size_t start = sa->len ;
  size_t refpos = start, uapos = start ;
  if (!(v & TIPIDEE_LOG_REQUEST)) return ;
  if (!string_quotes(sa, rql->uri.path)) goto eerr ;
  if (v & TIPIDEE_LOG_REFERRER)
  {
    refpos = sa->len ;
    if (!string_quotes(sa, referrer) || !stralloc_0(sa)) goto err ;
  }
  if (v & TIPIDEE_LOG_UA)
  {
    uapos = sa->len ;
    if (!string_quotes(sa, ua) || !stralloc_0(sa)) goto err ;
  }
  a[m++] = tipidee_method_tostr(rql->m) ;
  a[m++] = " host " ;
  a[m++] = rql->uri.host ;
  a[m++] = " path " ;
  a[m++] = sa->s + start ;
  if (rql->uri.query)
  {
    a[m++] = " query " ;
    a[m++] = rql->uri.query ;
  }
  a[m++] = " http 1." ;
  a[m++] = rql->http_minor ? "1" : "0" ;
  if (v & TIPIDEE_LOG_REFERRER)
  {
    a[m++] = " referrer " ;
    a[m++] = sa->s + refpos ;
  }
  if (v & TIPIDEE_LOG_UA)
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
