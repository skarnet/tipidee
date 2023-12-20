/* ISC license. */

#ifndef TIPIDEE_UTIL_H
#define TIPIDEE_UTIL_H

#include <sys/types.h>
#include <stdint.h>

#include <skalibs/uint64.h>
#include <skalibs/buffer.h>
#include <skalibs/tai.h>
#include <skalibs/stralloc.h>

typedef enum tipidee_transfercoding_e tipidee_transfercoding, *tipidee_transfercoding_ref ;
enum tipidee_transfercoding_e
{
  TIPIDEE_TRANSFERCODING_NONE = 0,
  TIPIDEE_TRANSFERCODING_FIXED,
  TIPIDEE_TRANSFERCODING_CHUNKED,
  TIPIDEE_TRANSFERCODING_UNKNOWN
} ;

typedef struct tipidee_defaulttext_s tipidee_defaulttext, *tipidee_defaulttext_ref ;
struct tipidee_defaulttext_s
{
  char const *reason ;
  char const *text ;
} ;

extern int tipidee_util_chunked_read (buffer *, stralloc *, size_t, tain const *, tain *) ;
#define tipidee_util_chunked_read_g(b, sa, max, deadline) tipidee_util_chunked_read(b, sa, max, (deadline), &STAMP)

extern int tipidee_util_httpdate (char const *, tain *) ;
extern int tipidee_util_defaulttext (unsigned int, tipidee_defaulttext *) ;
extern int tipidee_util_parse_range (char const *, off_t, uint64_t *, uint64_t *) ;

#endif
