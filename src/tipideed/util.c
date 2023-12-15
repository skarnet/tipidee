/* ISC license. */

#include <errno.h>
#include <string.h>

#include <skalibs/stralloc.h>
#include <skalibs/djbunix.h>

#include "tipideed-internal.h"

size_t translate_path (char const *path)
{
  size_t n = g.sa.len ;
  if (g.flagnoxlate)
  {
    if (!stralloc_readyplus(&g.sa, strlen(path) + 2)) return 0 ;
    path_canonicalize(g.sa.s + n, path, 0) ;
    return n ;
  }
  if (sarealpath(&g.sa, path) == -1 || !stralloc_0(&g.sa))
  {
    g.sa.len = n ;
    return 0 ;
  }
  g.sa.len = n ;
  if (strncmp(g.sa.s + n, g.sa.s, g.cwdlen) || g.sa.s[n + g.cwdlen] != '/')
    return (errno = EPERM, 0) ;
  return n + g.cwdlen + 1 ;
}
