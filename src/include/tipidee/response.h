/* ISC license. */

#ifndef TIPIDEE_RESPONSE_H
#define TIPIDEE_RESPONSE_H

#include <skalibs/bsdsnowflake.h>

#include <stddef.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/stat.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>

#include <tipidee/rql.h>
#include <tipidee/headers.h>

typedef struct tipidee_response_header_s tipidee_response_header, *tipidee_response_header_ref ;
struct tipidee_response_header_s
{
  char const *key ;
  char const *value ;
  uint8_t options ;
} ;

extern size_t tipidee_response_status (buffer *, tipidee_rql const *, unsigned int, char const *) ;

extern size_t tipidee_response_header_date_fmt (char *, size_t, tain const *) ;
extern size_t tipidee_response_header_date (char *, size_t, tain const *) ;
#define tipidee_response_header_date_g(buf, max) tipidee_response_header_date(buf, (max), &STAMP)
extern size_t tipidee_response_header_lastmodified (char *, size_t, struct stat const *) ;

extern size_t tipidee_response_header_writeall (buffer *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
#define tipidee_response_header_writeall_g(b, rhdr, rhdrn, options) tipidee_response_header_writeall(b, rhdr, rhdrn, (options), &STAMP)
extern size_t tipidee_response_header_writemerge (buffer *, tipidee_response_header const *, uint32_t, tipidee_headers const *, uint32_t, tain const *) ;
#define tipidee_response_header_writemerge_g(b, rhdr, rhdrn, hdr, options) tipidee_response_header_writemerge(b, rhdr, rhdrn, hdr, (options), &STAMP)

size_t tipidee_response_file (buffer *, tipidee_rql const *, unsigned int, char const *, struct stat const *, char const *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
#define tipidee_response_file_g(b, rql, status, reason, st, ct, rhdr, rhdrn, options) tipidee_response_file(b, rql, status, reason, st, ct, rhdr, rhdrn, (options), &STAMP)

extern size_t tipidee_response_error_nofile (buffer *, tipidee_rql const *, unsigned int, char const *, char const *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
#define tipidee_response_error_nofile_g(b, rql, status, reason, text, rhdr, rhdrn, options) tipidee_response_error_nofile(b, rql, status, reason, text, rhdr, rhdrn, (options), &STAMP)

extern int tipidee_response_header_preparebuiltin (tipidee_response_header *, uint32_t, char const *, size_t) ;

#endif
