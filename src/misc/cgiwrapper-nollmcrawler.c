/* ISC license. */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <regex.h>

#include <skalibs/posixplz.h>
#include <skalibs/bytestr.h>
#include <skalibs/stat.h>
#include <skalibs/prog.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/gol.h>
#include <skalibs/fmtscan.h>
#include <skalibs/exec.h>

#define USAGE "as a CGI script: cgiwrapper-nollmcrawler [ -v verbosity ] [ -d pathinfodepth ] rulesdir badregex realcgit..."
#define dieusage() strerr_dieusage(100, USAGE)

enum golb_e
{
  GOLB_FORCE = 0x01
} ;

static gol_bool const rgolb[] =
{
  { .so = 'f', .lo = "force", .clear = 0, .set = GOLB_FORCE }
} ;

enum gola_e
{
  GOLA_VERBOSITY,
  GOLA_DEPTH,
  GOLA_N
} ;

static gol_arg const rgola[] =
{
  { .so = 'v', .lo = "verbosity", .i = GOLA_VERBOSITY },
  { .so = 'd', .lo = "pathinfo-depth", .i = GOLA_DEPTH }
} ;

int main (int argc, char const *const *argv)
{
  unsigned int verbosity = 1 ;
  char const *wgola[GOLA_N] = { 0 } ;
  uint64_t wgolb = 0 ;
  unsigned int golc ;
  unsigned int depth = 0 ;
  char const *remoteaddr ;
  char const *x ;
  size_t rdlen, m = 0 ;
  char ip[16] ;
  int is6 ;
  PROG = "cgit-nollmcrawler" ;

  golc = GOL_main(argc, argv, rgolb, rgola, &wgolb, wgola) ;
  argc -= golc ; argv += golc ;
  if (wgola[GOLA_VERBOSITY] && !uint0_scan(wgola[GOLA_VERBOSITY], &verbosity))
    strerr_dief1x(100, "verbosity needs to be an unsigned integer") ;
  if (wgola[GOLA_DEPTH] && !uint0_scan(wgola[GOLA_DEPTH], &depth))
    strerr_dief1x(100, "pathinfo-depth needs to be an unsigned integer") ;
  if (argc < 3) dieusage() ;

  remoteaddr = getenv("REMOTE_ADDR") ;
  if (ip6_scan(remoteaddr, ip)) is6 = 1 ;
  else if (ip4_scan(remoteaddr, ip)) is6 = 0 ;
  else strerr_dieinvalid(100, "REMOTE_ADDR") ;

  rdlen = strlen(argv[0]) ;
  char fn[rdlen + IP6_FMT + 16] ;

  memcpy(fn + m, argv[0], rdlen) ; m += rdlen ;
  memcpy(fn + m, "/ip", 3) ; m += 3 ;
  fn[m++] = is6 ? '6' : '4' ;
  fn[m++] = '/' ;
  m += is6 ? ip6_fmt(fn + m, ip) : ip4_fmt(fn + m, ip) ;
  fn[m++] = '_' ;
  memcpy(fn + m, is6 ? "128" : "32", is6 ? 3 : 2) ; m += 2 + is6 ;
  fn[m] = 0 ;
  memcpy(fn + m, "/allow", 7) ;
  if (access(fn, W_OK) == 0) goto allow ;
  if (errno != ENOENT) strerr_diefu2sys(111, "access ", fn) ;
  memcpy(fn + m + 1, "deny", 5) ;
  if (access(fn, W_OK) == 0) goto deny ;
  if (errno != ENOENT) strerr_diefu2sys(111, "access ", fn) ;
  fn[m] = 0 ;

  if (depth)
  {
    x = getenv("PATH_INFO") ;
    if (!x) goto writeandallow ;
    if (byte_count(x, strlen(x), '/') <= depth) goto writeandallow ;
  }
  x = getenv("QUERY_STRING") ;
  if (!x) goto writeandallow ;

  {
    regex_t re ;
    int e = regcomp(&re, argv[1], REG_EXTENDED) ;
    if (e == REG_BADPAT) strerr_dief2x(100, "invalid regex: ", argv[1]) ;
    if (e)
    {
      char fmt[INT_FMT] ;
      fmt[int_fmt(fmt, e)] = 0 ;
      strerr_diefu4x(111, "regcomp ", argv[1], ": error code is ", fmt) ;
    }
    e = regexec(&re, x, 0, 0, REG_NOSUB) ;
    if (e == 0) goto writeanddeny ;
    if (e != REG_NOMATCH)
    {
      char fmt[INT_FMT] ;
      fmt[int_fmt(fmt, e)] = 0 ;
      strerr_diefu4x(111, "regexec ", argv[1], ": error code is ", fmt) ;
    }
    // regfree(&re) ;
  }

 writeandallow:
  if (symlink("../outputs/allow", fn) == -1)
  {
    if (errno != EEXIST) strerr_diefu4sys(111, "symlink ", "../outputs/allow", " to ", fn) ;
  }
  if (verbosity >= 2)
  {
    x = getenv("REQUEST_URI") ;
    strerr_warni("allowing ", remoteaddr, " requesting ", x) ;
  }
 allow:
  exec(argv + 2) ;
  strerr_dieexec(111, argv[2]) ;

 writeanddeny:
  if (symlink("../outputs/deny", fn) == -1)
  {
    if (errno != EEXIST) strerr_diefu4sys(111, "symlink ", "../outputs/deny", " to ", fn) ;
  }
 deny:
  buffer_putsflush(buffer_1small, "Status: 403 Go fuck yourself, crawler\nContent-Length: 0\n\n") ;
  _exit(0) ;
}
