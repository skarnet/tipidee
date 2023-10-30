/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <skalibs/uint32.h>
#include <skalibs/strerr.h>
#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>

#include <tipidee/config.h>
#include "tipidee-config-internal.h"

struct builtinheaders_s
{
  char const *key ;
  char const *value ;
  uint8_t overridable : 1 ;
} ;

static struct builtinheaders_s const builtinheaders[] =
{
  { .key = "Accept-Ranges", .value = "none", .overridable = 0 },
  { .key = "Allow", .value = 0, .overridable = 0 },
  { .key = "Cache-Control", .value = "private", .overridable = 1 },
  { .key = "Connection", .value = 0, .overridable = 0 },
  { .key = "Content-Security-Policy", .value = "default-src 'self'; style-src 'self' 'unsafe-inline';", .overridable = 1 },
  { .key = "Date", .value = 0, .overridable = 0 },
  { .key = "Referrer-Policy", .value = "no-referrer-when-downgrade", .overridable = 1 },
  { .key = "Server", .value = "tipidee/" TIPIDEE_VERSION, .overridable = 0 },
  { .key = "Status", .value = 0, .overridable = 0 },
  { .key = "Vary", .value = "Accept-Encoding", .overridable = 0 },
  { .key = "X-Content-Type-Options", .value = "nosniff", .overridable = 1 },
  { .key = "X-Frame-Options", .value = "DENY", .overridable = 1 }
} ;

static stralloc headers_storage = GENALLOC_ZERO ;

static repo headers = \
{ \
  .ga = GENALLOC_ZERO, \
  .tree = AVLTREE_INIT(8, 3, 8, &node_dtok, &node_cmp, &headers.ga), \
  .storage = &headers_storage \
} ;

int header_allowed (char const *key)
{
  static char const *const nope[] =
  {
    "Content-Length",
    "Content-Type",
    "Location"
  } ;
  struct builtinheaders_s const *p ;
  if (BSEARCH(char const *, key, nope)) return 0 ;
  p = BSEARCH(struct builtinheaders_s, key, builtinheaders) ;
  return !p || p->overridable ;
}

void header_canonicalize (char *key)
{
  int h = 1 ;
  size_t len = strlen(key) ;
  for (size_t i = 0 ; i < len ; i++)
  {
    key[i] = h ? toupper(key[i]) : tolower(key[i]) ;
    h = key[i] == '-' ;
  }
}

node const *headers_search (char const *key)
{
  return repo_search(&headers, key) ;
}

void headers_addv (char const *key, uint8_t options, char const *s, size_t const *word, size_t n, size_t filepos, uint32_t line)
{
  node node ;
  char c = !s << 7 | (options & 1) ;
  node_start(&headers_storage, &node, key, filepos, line) ;
  node_add(&headers_storage, &node, &c, 1) ;
  if (s)
  {
    node_add(&headers_storage, &node, s + word[0], strlen(s + word[0])) ;
    for (size_t i = 1 ; i < n ; i++)
    {
      node_add(&headers_storage, &node, " ", 1) ;
      node_add(&headers_storage, &node, s + word[i], strlen(s + word[i])) ;
    }
    node_add(&headers_storage, &node, "", 1) ;
  }
  repo_add(&headers, &node) ;
}

static inline void headers_defaults (void)
{
  for (size_t i = 0 ; i < sizeof(builtinheaders) / sizeof(struct builtinheaders_s) ; i++)
  {
    node node ;
    struct builtinheaders_s const *p = builtinheaders + i ;
    char c = (!p->value << 7) | p->overridable ;
    if (p->overridable && headers_search(p->key)) continue ;
    node_start(&headers_storage, &node, p->key, 0, 0) ;
    node_add(&headers_storage, &node, &c, 1) ;
    if (p->value) node_add(&headers_storage, &node, p->value, strlen(p->value) + 1) ;
    repo_add(&headers, &node) ;
  }
}

static int header_write (uint32_t d, unsigned int h, void *data)
{
  node *confnode = data ;
  node *const header = genalloc_s(node, &headers.ga) + d ;
  (void)h ;
  confnode_add(confnode, headers_storage.s + header->key, header->keylen + 1) ;
  confnode_add(confnode, headers_storage.s + header->data, header->datalen) ;
  return 1 ;
}

void headers_finish (void)
{
  node node ;
  char pack[4] ;
  headers_defaults() ;
  uint32_pack_big(pack, avltree_len(&headers.tree)) ;
  confnode_start(&node, "G:response_headers", 0, 0) ;
  (void)avltree_iter(&headers.tree, &header_write, &node) ;
  confnode_add(&node, pack, 4) ;
  conftree_add(&node) ;
  avltree_free(&headers.tree) ;
  genalloc_free(node, &headers.ga) ;
  stralloc_free(&headers_storage) ;
}
