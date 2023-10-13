/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <skalibs/gensetdyn.h>
#include <skalibs/avltree.h>
#include <skalibs/cdbmake.h>
#include <skalibs/strerr.h>

#include "tipidee-config-internal.h"

static void *confnode_dtok (uint32_t d, void *data)
{
  return g.storage.s + GENSETDYN_P(confnode, (gensetdyn *)data, d)->key ;
}

static int confnode_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  return strcmp((char const *)a, (char const *)b) ;
}

struct nodestore_s
{
  gensetdyn set ;
  avltree tree ;
} ;

static struct nodestore_s nodestore = \
{ \
  .set = GENSETDYN_INIT(confnode, 8, 3, 8), \
  .tree = AVLTREE_INIT(8, 3, 8, &confnode_dtok, &confnode_cmp, &nodestore.set) \
} ;

confnode const *conftree_search (char const *key)
{
  uint32_t i ;
  return avltree_search(&nodestore.tree, key, &i) ? GENSETDYN_P(confnode const, &nodestore.set, i) : 0 ;
}

void conftree_add (confnode const *node)
{
  uint32_t i ;
  if (!gensetdyn_new(&nodestore.set, &i)) dienomem() ;
  *GENSETDYN_P(confnode, &nodestore.set, i) = *node ;
  if (!avltree_insert(&nodestore.tree, i)) dienomem() ;
}

void conftree_update (confnode const *node)
{
  uint32_t i ;
  if (avltree_search(&nodestore.tree, g.storage.s + node->key, &i))
  {
    if (!avltree_delete(&nodestore.tree, g.storage.s + node->key)) dienomem() ;
    *GENSETDYN_P(confnode, &nodestore.set, i) = *node ;
    if (!avltree_insert(&nodestore.tree, i)) dienomem() ;
  }
  else return conftree_add(node) ;
}

static int confnode_write (uint32_t d, unsigned int h, void *data)
{
  confnode *node = GENSETDYN_P(confnode, &nodestore.set, d) ;
  (void)h ;
  if ((g.storage.s[node->key] & ~0x20) == 'A')
  {
    g.storage.s[++node->data] |= '@' ;
    node->datalen-- ;
  }
  return cdbmake_add((cdbmaker *)data, g.storage.s + node->key, node->keylen, g.storage.s + node->data, node->datalen) ;
}

int conftree_write (cdbmaker *cm)
{
  return avltree_iter(&nodestore.tree, &confnode_write, cm) ;
}
