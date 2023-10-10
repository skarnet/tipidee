/* ISC license. */

#include <sys/types.h>

#include <skalibs/uint64.h>
#include <skalibs/types.h>
#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_answer (uint32_t v, unsigned int status, off_t size)
{
  char fmtstatus[UINT_FMT] ;
  if (!(v & TIPIDEE_LOG_ANSWER)) return ;
  fmtstatus[uint_fmt(fmtstatus, status)] = 0 ;
  if (size)
  {
    char fmtsize[UINT64_FMT] ;
    fmtsize[uint64_fmt(fmtsize, size)] = 0 ;
    strerr_warni4x("answer ", fmtstatus, " size ", fmtsize) ;
  }
  else
    strerr_warni2x("answer ", fmtstatus) ;
}
