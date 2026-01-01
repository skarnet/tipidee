/* ISC license. */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>  /* rename() */
#include <errno.h>
#include <signal.h>

#include <skalibs/posixplz.h>
#include <skalibs/types.h>
#include <skalibs/sgetopt.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/sig.h>
#include <skalibs/cspawn.h>
#include <skalibs/djbunix.h>

#include <tipidee/config.h>
#include "tipidee-config-internal.h"

#define USAGE "tipidee-config [ -i textfile ] [ -o cdbfile ] [ -m mode ]"
#define dieusage() strerr_dieusage(100, USAGE)

struct global_s g = GLOBAL_ZERO ;

static pid_t pid = 0 ;

static void sigchld_handler (int sig)
{
  int e = errno ;
  for (;;)
  {
    int wstat ;
    pid_t r = wait_nohang(&wstat) ;
    if (r == -1 && errno != ECHILD) strerr_diefu1sys(111, "wait") ;
    else if (r <= 0) break ;
    else if (r == pid)
    {
      if (WIFEXITED(wstat) && !WEXITSTATUS(wstat)) pid = 0 ;
      else _exit(wait_estatus(wstat)) ;
    }
  }
  (void)sig ;
  errno = e ;
}

static inline void conf_output (char const *ofile, unsigned int omode)
{
  int fdw ;
  cdbmaker cm = CDBMAKER_ZERO ;
  size_t olen = strlen(ofile) ;
  char otmp[olen + 8] ;
  memcpy(otmp, ofile, olen) ;
  memcpy(otmp + olen, ":XXXXXX", 8) ;
  fdw = mkstemp(otmp) ;
  if (fdw == -1) strerr_diefu3sys(111, "open ", otmp, " for writing") ;
  if (!cdbmake_start(&cm, fdw))
  {
    unlink_void(otmp) ;
    strerr_diefu2sys(111, "cdmake_start ", otmp) ;
  }
  if (!conftree_write(&cm))
  {
    unlink_void(otmp) ;
    strerr_diefu2sys(111, "write config tree into ", otmp) ;
  }
  if (!cdbmake_finish(&cm))
  {
    unlink_void(otmp) ;
    strerr_diefu2sys(111, "cdbmake_finish ", otmp) ;
  }
  if (fsync(fdw) == -1)
  {
    unlink_void(otmp) ;
    strerr_diefu2sys(111, "fsync ", otmp) ;
  }
  if (fchmod(fdw, omode & 0777) == -1)
  {
    unlink_void(otmp) ;
    strerr_diefu2sys(111, "fchmod ", otmp) ;
  }
  if (rename(otmp, ofile) == -1)
  {
    unlink_void(otmp) ;
    strerr_diefu4sys(111, "rename ", otmp, " to ", ofile) ;
  }
}

int main (int argc, char const *const *argv, char const *const *envp)
{
  char const *ifile = TIPIDEE_SYSCONFPREFIX "tipidee.conf" ;
  char const *ofile = TIPIDEE_SYSCONFPREFIX "tipidee.conf.cdb" ;
  unsigned int omode = 0644 ;

  PROG = "tipidee-config" ;
  {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, argv, "i:o:m:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        case 'i' : ifile = l.arg ; break ;
        case 'o' : ofile = l.arg ; break ;
        case 'm' : if (!uint0_oscan(l.arg, &omode)) dieusage() ; break ;
        default : strerr_dieusage(100, USAGE) ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }

  {
    int fdr ;
    buffer b ;
    char buf[4096] ;
    sig_block(SIGCHLD) ;
    if (!sig_catch(SIGCHLD, &sigchld_handler))
      strerr_diefu1sys(111, "install SIGCHLD handler") ;
    {
      char const *ppargv[3] = { TIPIDEE_LIBEXECPREFIX "tipidee-config-preprocess", ifile, 0 } ;
      pid = child_spawn1_pipe(ppargv[0], ppargv, envp, &fdr, 1) ;
      if (!pid) strerr_diefu2sys(errno == ENOENT ? 127 : 126, "spawn ", ppargv[0]) ;
    }
    sig_unblock(SIGCHLD) ;
    buffer_init(&b, &buffer_read, fdr, buf, 4096) ;
    conf_lexparse(&b, ifile) ;
  }
  conf_defaults() ;
  conf_output(ofile, omode) ;
  return 0 ;
}
