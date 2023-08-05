/* ISC license. */

#ifndef TIPIDEE_HEADERS_H
#define TIPIDEE_HEADERS_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/disize.h>
#include <skalibs/buffer.h>
#include <skalibs/tai.h>
#include <skalibs/avltreen.h>

#define TIPIDEE_HEADERS_MAX 32

typedef struct tipidee_headers_s tipidee_headers, *tipidee_headers_ref ;
struct tipidee_headers_s
{
  char *buf ;
  size_t max ;
  size_t len ;
  disize list[TIPIDEE_HEADERS_MAX] ;
  avltreen map ;
  uint32_t map_freelist[TIPIDEE_HEADERS_MAX] ;
  avlnode map_storage[TIPIDEE_HEADERS_MAX] ;
  uint32_t n ;
} ;

extern void tipidee_headers_init (tipidee_headers *, char *, size_t) ;

extern int tipidee_headers_parse_nb (buffer *, tipidee_headers *, disize *, uint32_t *) ;
extern int tipidee_headers_timed_parse (buffer *, tipidee_headers *, tain const *, tain *) ;
#define tipidee_headers_timed_parse_g(b, hdr, deadline) tipidee_headers_timed_parse(b, hdr, (deadline), &STAMP)

extern char const *tipidee_headers_search (tipidee_headers const *, char const *) ;
extern ssize_t tipidee_headers_get_content_length (tipidee_headers const *) ;

#endif
