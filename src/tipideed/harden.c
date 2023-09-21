/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#include <skalibs/sysdeps.h>

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#ifdef SKALIBS_HASSETGROUPS
#include <grp.h>
#endif

#include <skalibs/types.h>
#include <skalibs/strerr.h>

#include "tipideed-internal.h"

static inline void tipideed_chroot (void)
{
#ifdef SKALIBS_HASCHROOT
  if (chroot(".") == -1) strerr_diefu1sys(111, "chroot") ;
#else
  errno = ENOSYS ;
  strerr_warnwu1sys("chroot") ; 
#endif
}

static inline void tipideed_dropuidgid (void)
{
  uid_t uid = 0 ;
  gid_t gid = 0 ;
  char const *gidfmt = getenv("GID") ;
  char const *uidfmt = getenv("UID") ;
  if (!uidfmt) strerr_dienotset(100, "UID") ;
  if (!uid0_scan(uidfmt, &uid)) strerr_dieinvalid(100, "UID") ;
  if (!gidfmt) strerr_dienotset(100, "GID") ;
  if (!gid0_scan(gidfmt, &gid)) strerr_dieinvalid(100, "GID") ;
  if (gid)
  {
#ifdef SKALIBS_HASSETGROUPS
    if (setgroups(1, &gid) == -1) strerr_diefu2sys(111, "setgroups to ", gidfmt) ;
#endif
    if (setgid(gid) == -1) strerr_diefu2sys(111, "setgid to ", gidfmt) ;
  }
  if (uid)
    if (setuid(uid) == -1) strerr_diefu2sys(111, "setuid to ", uidfmt) ;
}

void tipideed_harden (unsigned int h)
{
  if (h & 2) tipideed_chroot() ;
  if (h & 1) tipideed_dropuidgid() ;
}
