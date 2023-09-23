/* ISC license. */

#include <unistd.h>

#include <skalibs/uint16.h>
#include <skalibs/types.h>
#include <skalibs/strerr.h>

#include <tipidee/method.h>
#include "tipideed-internal.h"

void log_start (void)
{
  if (g.verbosity >= 4)
    strerr_warni7x("new connection", " from ip ", g.sa.s + g.remoteip, " (", g.sa.s + g.remotehost, ") port ", g.sa.s + g.remoteport) ;
  else if (g.verbosity >= 3)
    strerr_warni1x("new connection") ;
}

void log_and_exit (int e)
{
  if (g.verbosity >= 3)
  {
    char fmt[INT_FMT] ;
    fmt[int_fmt(fmt, e)] = 0 ;
    strerr_warni2x("exiting ", fmt) ;
  }
  _exit(e) ;
}

void log_request (tipidee_rql const *rql)
{
  if (g.verbosity >= 2)
  {
    char fmt[UINT16_FMT] ;
    if (rql->uri.port) fmt[uint16_fmt(fmt, rql->uri.port)] = 0 ;
    strerr_warnin(11, "request ", tipidee_method_tostr(rql->m), " for", rql->uri.host ? " host " : "", rql->uri.host ? rql->uri.host : "", rql->uri.port ? " port " : "", rql->uri.port ? fmt : "", " path ", rql->uri.path, rql->uri.query ? " query " : "", rql->uri.query ? rql->uri.query : "") ;
  }
}

void log_regular (char const *fn, char const *sizefmt, int ishead, char const *ct)
{
  if (g.verbosity >= 2)
    strerr_warni8x("sending ", ishead ? "headers for " : "", "regular file ", fn, " (", sizefmt, " bytes) with type ", ct) ;
}

void log_response (char const *ans, char const *fn)
{
  if (g.verbosity >= 2)
    strerr_warni4x("answering ", ans, fn ? " for " : 0, fn) ;
}

void log_nph (char const *const *argv, char const *const *envp)
{
  if (g.verbosity >= 2)
    strerr_warni3x("running ", "nph ", argv[0]) ;
  (void)envp ;
}

void log_cgi (char const *const *argv, char const *const *envp)
{
  if (g.verbosity >= 2)
    strerr_warni3x("running ", "cgi ", argv[0]) ;
  (void)envp ;
}
