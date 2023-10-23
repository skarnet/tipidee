/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/stralloc.h>
#include <skalibs/strerr.h>

#include "tipidee-config-internal.h"

#define diestorage() strerr_diefu2x(100, "add node to configuration tree", ": too much data")
#define diefilepos() strerr_diefu2x(100, "add node to configuration tree", ": file too large")

void node_start (stralloc *storage, node *node, char const *key, size_t filepos, uint32_t line)
{
  size_t l = strlen(key) ;
  size_t k = storage->len ;
  if (!stralloc_catb(storage, key, l + 1)) dienomem() ;
  if (storage->len >= UINT32_MAX) diestorage() ;
  if (filepos > UINT32_MAX) diefilepos() ;
  node->key = k ;
  node->keylen = l ;
  node->data = storage->len ;
  node->datalen = 0 ;
  node->filepos = filepos ;
  node->line = line ;
}

void node_add (stralloc *storage, node *node, char const *s, size_t len)
{
  if (!stralloc_catb(storage, s, len)) dienomem() ;
  if (storage->len >= UINT32_MAX) diestorage() ;
  node->datalen += len ;
}
