/* ISC license. */

#ifndef TIPIDEE_FCGI_H
#define TIPIDEE_FCGI_H

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <skalibs/buffer.h>
#include <skalibs/tai.h>
#include <skalibs/stralloc.h>

typedef struct tipidee_fcgi_s tipidee_fcgi, *tipidee_fcgi_ref ;
struct tipidee_fcgi_s
{
  buffer b ;
  char buf[4096] ;
  tain deadline ;
  uint16_t id ;
} ;
#define TIPIDEE_FCGI_ZERO { .b = BUFFER_ZERO, .deadline = TAIN_ZERO, .id = 0 }

extern void tipidee_fcgi_startwrite (tipidee_fcgi *, int, pid_t, tain const *) ;
extern void tipidee_fcgi_startread (tipidee_fcgi *, int, pid_t, tain const *) ;

extern size_t tipidee_fcgi_put (tipidee_fcgi *, uint8_t, char const *, size_t, tain *) ;
#define tipidee_fcgi_put_g(fc, rectype, s, len) tipidee_fcgi_put(fc, rectype, s, (len), &STAMP)

extern size_t tipidee_fcgi_putv (tipidee_fcgi *, uint8_t, struct iovec const *, unsigned int, tain *) ;
#define tipidee_fcgi_putv_g(fc, rectype, v, n) tipidee_fcgi_putv(fc, rectype, v, (n), &STAMP)

extern int tipidee_fcgi_flush (tipidee_fcgi *, tain *) ;
#define tipidee_fcgi_flush_g(fc) tipidee_fcgi_flush((fc), &STAMP)

extern int tipidee_fcgi_beginrequest (tipidee_fcgi *, tain *) ;
#define tipidee_fcgi_beginrequest_g(fc) tipidee_fcgi_beginrequest((fc), &STAMP)

extern int tipidee_fcgi_addenvb (stralloc *, char const *, size_t, char const *, size_t) ;
#define tipidee_fcgi_addenv(sa, key, val) tipidee_fcgi_addenvb(sa, key, strlen(key), val, strlen(val))

extern int tipidee_fcgi_params (tipidee_fcgi *, char const *, size_t, tain *) ;
#define tipidee_fcgi_params_g(fc, s, len) tipidee_fcgi_params(fc, s, (len), &STAMP)

extern int tipidee_fcgi_stdin (tipidee_fcgi *, char const *, size_t, tain *) ;
#define tipidee_fcgi_stdin_g(fc, s, len) tipidee_fcgi_stdin(fc, s, (len), &STAMP)

extern int tipidee_fcgi_read (tipidee_fcgi *, uint32_t *, stralloc *, tain *) ;
#define tipidee_fcgi_read_g(fc, status, sa) tipidee_fcgi_read(fc, status, (sa), &STAMP)

typedef enum fcgi_type_e fcgi_type ;
enum fcgi_type_e
{
  FCGI_BEGIN_REQUEST = 1,
  FCGI_ABORT_REQUEST,
  FCGI_END_REQUEST,
  FCGI_PARAMS,
  FCGI_STDIN,
  FCGI_STDOUT,
  FCGI_STDERR,
  FCGI_DATA,
  FCGI_GET_VALUES,
  FCGI_GET_VALUES_RESULT,
  FCGI_UNKNOWN_TYPE,
} ;

#define FCGI_VERSION_1 1

#define FCGI_NULL_REQUEST_ID 0

#define FCGI_KEEP_CONN  1

#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

#endif
