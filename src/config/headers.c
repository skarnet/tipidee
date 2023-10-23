/* ISC license. */

#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>

#include "tipidee-config-internal.h"

static stralloc headers_storage = GENALLOC_ZERO ;

static repo headers = \
{ \
  .ga = GENALLOC_ZERO, \
  .tree = AVLTREE_INIT(8, 3, 8, &node_dtok, &node_cmp, &headers.ga), \
  .storage = &headers_storage \
} ;

void header_start (node *node, char const *key, size_t filepos, uint32_t line)
{
  return node_start(&headers_storage, node, key, filepos, line) ;
}

void header_add (node *node, char const *s, size_t len)
{
  return node_add(&headers_storage, node, s, len) ;
}

node const *headers_search (char const *key)
{
  return repo_search(&headers, key) ;
}

void headers_add (node const *node)
{
  return repo_add(&headers, node) ;
}

static int header_write (uint32_t d, unsigned int h, void *data)
{
  return 1 ;
}

int headers_write (void)
{
  if (!avltree_iter(&headers.tree, &header_write, 0)) return 0 ;
  return 1 ;
}
