/* ISC license. */

#include <skalibs/strerr.h>

#include <tipidee/log.h>

void tipidee_log_resource (uint32_t v, char const *docroot, char const *file, tipidee_resattr const *ra)
{
  if (!(v & TIPIDEE_LOG_RESOURCE)) return ;
  strerr_warni6x("docroot ", docroot, " file ", file, " type ", ra->iscgi ? ra->isnph ? "nph" : "cgi" : ra->content_type) ;
}
