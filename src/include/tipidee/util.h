/* ISC license. */

#ifndef TIPIDEE_UTIL_H
#define TIPIDEE_UTIL_H

#include <stddef.h>
#include <stdint.h>

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

extern int tipidee_util_chunked_read (buffer *, stralloc *, size_t, tain const *, tain *) ;
#define tipidee_util_chunked_read_g(b, sa, max, deadline) tipidee_util_chunked_read(b, sa, max, (deadline), &STAMP)

extern int tipidee_util_httpdate (char const *, tain *) ;

#endif
