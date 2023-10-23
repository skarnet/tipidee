/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>

#include "tipidee-config-internal.h"

void *node_dtok (uint32_t d, void *data)
{
  repo *r = data ;
  return r->storage->s + genalloc_s(node, &r->ga)[d].key ;
}

int node_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  return strcmp((char const *)a, (char const *)b) ;
}

node const *repo_search (repo const *r, char const *key)
{
  uint32_t i ;
  return avltree_search(&r->tree, key, &i) ? genalloc_s(node const, &r->ga) + i : 0 ;
}

void repo_add (repo *r, node const *node)
{
  uint32_t i = genalloc_len(node, &r->ga) ;
  if (!genalloc_append(node, &r->ga, node)) dienomem() ;
  if (!avltree_insert(&r->tree, i)) dienomem() ;
}

void repo_update (repo *r, node const *nod)
{
  uint32_t i ;
  if (avltree_search(&r->tree, r->storage->s + nod->key, &i))
  {
    if (!avltree_delete(&r->tree, r->storage->s + nod->key)) dienomem() ;
    genalloc_s(node, &r->ga)[i] = *nod ;
    if (!avltree_insert(&r->tree, i)) dienomem() ;
  }
  else repo_add(r, nod) ;
}
