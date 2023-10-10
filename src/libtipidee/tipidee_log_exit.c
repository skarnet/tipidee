/* ISC license. */

#include <skalibs/types.h>
#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_exit (uint32_t v, unsigned int e)
{
  char fmt[UINT_FMT] ;
  if (!(v & TIPIDEE_LOG_START)) return ;
  fmt[uint_fmt(fmt, e)] = 0 ;
  strerr_warni2x("exit ", fmt) ;
}
