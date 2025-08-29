/* ISC license. */

#ifndef TIPIDEE_RESATTR_H
#define TIPIDEE_RESATTR_H

#include <stdint.h>

 /* These values must match the bitattrs in lexparse.c */

#define TIPIDEE_RA_FLAG_CGI 0x0001
#define TIPIDEE_RA_FLAG_NPH 0x0002
#define TIPIDEE_RA_FLAG_BA 0x0004
#define TIPIDEE_RA_FLAG_AUTOCHUNK 0x0008
#define TIPIDEE_RA_FLAG_REALTIME 0x0010

#define TIPIDEE_RA_BITS 5

typedef struct tipidee_resattr_s tipidee_resattr, *tipidee_resattr_ref ;
struct tipidee_resattr_s
{
  uint32_t flags ;
  uint32_t mask ;
  char const *content_type ;
} ;
#define TIPIDEE_RESATTR_ZERO { .flags = 0, .mask = 0, .content_type = 0 }

#endif
