/* ISC license. */

#ifndef TIPIDEE_LOG_H
#define TIPIDEE_LOG_H

#include <stdint.h>

#include <skalibs/uint64.h>
#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>

#include <tipidee/rql.h>
#include <tipidee/headers.h>
#include <tipidee/resattr.h>

#define TIPIDEE_LOG_REQUEST 0x0001
#define TIPIDEE_LOG_REFERRER 0x0002
#define TIPIDEE_LOG_UA 0x0004
#define TIPIDEE_LOG_RESOURCE 0x0008
#define TIPIDEE_LOG_ANSWER 0x0010
#define TIPIDEE_LOG_SIZE 0x0020
#define TIPIDEE_LOG_START 0x0040
#define TIPIDEE_LOG_CLIENTIP 0x0080
#define TIPIDEE_LOG_CLIENTHOST 0x0100
#define TIPIDEE_LOG_HOSTASPREFIX 0x0200
#define TIPIDEE_LOG_XFORWARDEDFOR 0x0400
#define TIPIDEE_LOG_DEBUG 0x8000

#define TIPIDEE_LOG_DEFAULT (TIPIDEE_LOG_REQUEST | TIPIDEE_LOG_ANSWER | TIPIDEE_LOG_SIZE)

extern void tipidee_log_start (uint32_t, char const *, char const *) ;
extern void tipidee_log_exit (uint32_t, unsigned int) ;

extern void tipidee_log_request (uint32_t, tipidee_rql const *, tipidee_headers const *, stralloc *) ;
extern void tipidee_log_resource (uint32_t, tipidee_rql const *, char const *, tipidee_resattr const *, char const *) ;
extern void tipidee_log_answer (uint32_t, tipidee_rql const *, unsigned int, uint64_t) ;

#define tipidee_log_debug(v, ...) do { if ((v) & TIPIDEE_LOG_DEBUG) strerr_warn(PROG, ": debug: ", __VA_ARGS__) ; } while (0)

#endif
