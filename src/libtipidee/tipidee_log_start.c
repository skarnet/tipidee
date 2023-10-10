/* ISC license. */

#include <stddef.h>

#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_start (uint32_t v, char const *ip, char const *host)
{
  char const *a[5] = { "info: start" } ;
  size_t m = 1 ;
  if (!(v & TIPIDEE_LOG_START)) return ;
  if (v & TIPIDEE_LOG_CLIENTIP)
  {
    a[m++] = " ip " ;
    a[m++] = ip ;
  }
  if (v & TIPIDEE_LOG_CLIENTHOST)
  {
    a[m++] = " host " ;
    a[m++] = host ;
  }
  strerr_warnv(a, m) ;
}
