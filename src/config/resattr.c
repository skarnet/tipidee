/* ISC license. */

#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>
#include <skalibs/cdbmake.h>

#include "tipidee-config-internal.h"

static repo resattr = \
{ \
  .ga = GENALLOC_ZERO, \
  .tree = AVLTREE_INIT(8, 3, 8, &node_dtok, &node_cmp, &resattr.ga), \
  .storage = &g.storage \
} ;

void confnode_start (node *node, char const *key, size_t filepos, uint32_t line)
{
  return node_start(&g.storage, node, key, filepos, line) ;
}

void confnode_add (node *node, char const *s, size_t len)
{
  return node_add(&g.storage, node, s, len) ;
}

node const *conftree_search (char const *key)
{
  return repo_search(&conftree, key) ;
}

void conftree_add (node const *node)
{
  return repo_add(&conftree, node) ;
}

void conftree_update (node const *node)
{
  return repo_update(&conftree, node) ;
}

static int confnode_write (uint32_t d, unsigned int h, void *data)
{
  node *nod = genalloc_s(node, &conftree.ga) + d ;
  (void)h ;
  if ((conftree.storage->s[nod->key] & ~0x20) == 'A')
  {
    conftree.storage->s[++nod->data] |= '@' ;
    nod->datalen-- ;
  }
  return cdbmake_add((cdbmaker *)data, conftree.storage->s + nod->key, nod->keylen, conftree.storage->s + nod->data, nod->datalen) ;
}

int conftree_write (cdbmaker *cm)
{
  return avltree_iter(&conftree.tree, &confnode_write, cm) ;
}
