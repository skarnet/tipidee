/* ISC license. */

#include <sys/types.h>

#include <skalibs/uint64.h>
#include <skalibs/types.h>
#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_answer (uint32_t v, tipidee_rql const *rql, unsigned int status, off_t size)
{
  char const *a[6] = { PROG, ": info:" } ;
  size_t m = 2 ;
  char fmtstatus[UINT_FMT] ;
  char fmtsize[UINT64_FMT] ;
  if (!(v & TIPIDEE_LOG_ANSWER)) return ;
  if (v & TIPIDEE_LOG_HOSTASPREFIX)
  {
    a[m++] = " host " ;
    a[m++] = rql->uri.host ;
  }
  fmtstatus[uint_fmt(fmtstatus, status)] = 0 ;
  a[m++] = " answer " ;
  a[m++] = fmtstatus ;
  if (size)
  {
    fmtsize[uint64_fmt(fmtsize, size)] = 0 ;
    a[m++] = " size " ;
    a[m++] = fmtsize ;
  }
  strerr_warnv(a, m) ;
}
