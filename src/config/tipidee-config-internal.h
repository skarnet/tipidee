/* ISC license. */

#ifndef TIPIDEE_CONFIG_INTERNAL_H
#define TIPIDEE_CONFIG_INTERNAL_H

#include <stdint.h>
#include <string.h>

#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/cdbmake.h>

#define dienomem() strerr_diefu1sys(111, "stralloc_catb")

typedef struct confnode_s confnode, *confnode_ref ;
struct confnode_s
{
  uint32_t key ;
  uint32_t keylen ;
  uint32_t data ;
  uint32_t datalen ;
  uint32_t filepos ;
  uint32_t line ;
} ;
#define CONFNODE_ZERO { .key = 0, .keylen = 0, .data = 0, .datalen = 0 }

struct global_s
{
  stralloc storage ;
} ;
#define GLOBAL_ZERO { .storage = STRALLOC_ZERO }

extern struct global_s g ;


 /* confnode */

extern void confnode_start (confnode *, char const *, size_t, uint32_t) ;
extern void confnode_add (confnode *, char const *, size_t) ;


 /* conftree */

extern confnode const *conftree_search (char const *) ;
extern void conftree_add (confnode const *) ;
extern void conftree_update (confnode const *) ;
extern int conftree_write (cdbmaker *) ;


 /* lexparse */

extern void conf_lexparse (buffer *, char const *) ;


 /* defaults */

extern void conf_defaults (void) ;

#endif
