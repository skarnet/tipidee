/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/stralloc.h>
#include <skalibs/strerr.h>

#include "tipidee-config-internal.h"

#define diestorage() strerr_diefu2x(100, "add node to configuration tree", ": too much data")
#define diefilepos() strerr_diefu2x(100, "add node to configuration tree", ": file too large")

void confnode_start (confnode *node, char const *key, size_t filepos, uint32_t line)
{
  size_t l = strlen(key) ;
  size_t k = g.storage.len ;
  if (!stralloc_catb(&g.storage, key, l + 1)) dienomem() ;
  if (g.storage.len >= UINT32_MAX) diestorage() ;
  if (filepos > UINT32_MAX) diefilepos() ;
  node->key = k ;
  node->keylen = l ;
  node->data = g.storage.len ;
  node->datalen = 0 ;
  node->filepos = filepos ;
  node->line = line ;
}

void confnode_add (confnode *node, char const *s, size_t len)
{
  if (!stralloc_catb(&g.storage, s, len)) dienomem() ;
  if (g.storage.len >= UINT32_MAX) strerr_diefu1x(100, "add node to configuration tree: too much data") ;
  node->datalen += len ;
}
