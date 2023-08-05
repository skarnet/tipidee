/* ISC license. */

#ifndef TIPIDEE_URI_H
#define TIPIDEE_URI_H

#include <stddef.h>
#include <stdint.h>

typedef struct tipidee_uri_s tipidee_uri, *tipidee_uri_ref ;
struct tipidee_uri_s
{
  char const *host ;
  char const *path ;
  char const *query ;
  size_t lastslash ;
  uint16_t port ;
  uint8_t https : 1 ;
} ;
#define TIPIDEE_URI_ZERO \
{ \
  .host = 0, \
  .path = 0, \
  .query = 0, \
  .lastslash = 0, \
  .port = 0, \
  .https = 0 \
}

extern size_t tipidee_uri_parse (char *, size_t, char const *, tipidee_uri *) ;

#endif
