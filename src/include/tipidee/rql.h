/* ISC license. */

#ifndef TIPIDEE_RQL_H
#define TIPIDEE_RQL_H

#include <skalibs/buffer.h>
#include <skalibs/tai.h>

#include <tipidee/method.h>
#include <tipidee/uri.h>

typedef struct tipidee_rql_s tipidee_rql, *tipidee_rql_ref ;
struct tipidee_rql_s
{
  tipidee_method m ;
  unsigned int http_major ;
  unsigned int http_minor ;
  tipidee_uri uri ;
} ;
#define TIPIDEE_RQL_ZERO \
{ \
  .m = TIPIDEE_METHOD_UNKNOWN, \
  .http_major = 0, \
  .http_minor = 0, \
  .uri = TIPIDEE_URI_ZERO \
}

extern int tipidee_rql_read (buffer *, char *, size_t, size_t *, tipidee_rql *, tain const *, tain *) ;
#define tipidee_rql_read_g(b, buf, max, w, rql, deadline) tipidee_rql_read(b, buf, max, w, rql, (deadline), &STAMP)

#endif
