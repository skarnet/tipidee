/* ISC license. */

#ifndef TIPIDEE_LOG_H
#define TIPIDEE_LOG_H

#include <sys/types.h>
#include <stdint.h>

#include <skalibs/stralloc.h>

#include <tipidee/rql.h>

#define TIPIDEE_LOG_REQUEST 0x0001
#define TIPIDEE_LOG_REFERRER 0x0002
#define TIPIDEE_LOG_UA 0x0004
#define TIPIDEE_LOG_RESOURCE 0x0008
#define TIPIDEE_LOG_ANSWER 0x0010
#define TIPIDEE_LOG_SIZE 0x0020
#define TIPIDEE_LOG_START 0x100
#define TIPIDEE_LOG_CLIENTIP 0x0200
#define TIPIDEE_LOG_CLIENTHOST 0x0400
#define TIPIDEE_LOG_HOSTASPREFIX 0x1000

#define TIPIDEE_LOG_DEFAULT (TIPIDEE_LOG_REQUEST | TIPIDEE_LOG_ANSWER | TIPIDEE_LOG_SIZE)

typedef struct tipidee_resattr_s tipidee_resattr, *tipidee_resattr_ref ;
struct tipidee_resattr_s
{
  char const *content_type ;
  uint32_t iscgi : 1 ;
  uint32_t isnph : 1 ;
} ;
#define TIPIDEE_RESATTR_ZERO { .content_type = 0, .iscgi = 0, .isnph = 0 }

extern void tipidee_log_start (uint32_t, char const *, char const *) ;
extern void tipidee_log_exit (uint32_t, unsigned int) ;

extern void tipidee_log_request (uint32_t, tipidee_rql const *, char const *, char const *, stralloc *) ;
extern void tipidee_log_resource (uint32_t, tipidee_rql const *, char const *, char const *, tipidee_resattr const *) ;
extern void tipidee_log_answer (uint32_t, tipidee_rql const *, unsigned int, off_t) ;

#endif
