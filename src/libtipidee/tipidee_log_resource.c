/* ISC license. */

#include <stddef.h>

#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_resource (uint32_t v, tipidee_rql const *rql, char const *docroot, char const *file, tipidee_resattr const *ra)
{
  char const *a[8] = { PROG, ": info:" } ;
  size_t m = 2 ;
  if (!(v & TIPIDEE_LOG_RESOURCE)) return ;
  if (v & TIPIDEE_LOG_HOSTASPREFIX)
  {
    a[m++] = " host " ;
    a[m++] = rql->uri.host ;
  }
  a[m++] = " resource docroot " ;
  a[m++] = docroot ;
  a[m++] = " file " ;
  a[m++] = file ;
  a[m++] = " type " ;
  a[m++] = ra->iscgi ? ra->isnph ? "nph" : "cgi" : ra->content_type ;
  strerr_warnv(a, m) ;
}
