/* ISC license. */

#ifndef TIPIDEE_RESPONSE_H
#define TIPIDEE_RESPONSE_H

#include <skalibs/bsdsnowflake.h>

#include <stddef.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/stat.h>
#include <skalibs/uint64.h>
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
extern size_t tipidee_response_header_date_G (char *, size_t) ;
#define tipidee_response_header_date_g(buf, max) tipidee_response_header_date(buf, (max), &STAMP)

extern size_t tipidee_response_header_lastmodified (char *, size_t, struct stat const *) ;

extern size_t tipidee_response_header_write (buffer *, tipidee_response_header const *, uint32_t) ;
extern size_t tipidee_response_header_writeall (buffer *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
extern size_t tipidee_response_header_writeall_G (buffer *, tipidee_response_header const *, uint32_t, uint32_t) ;

extern size_t tipidee_response_header_end (buffer *) ;


extern size_t tipidee_response_header_writemerge (buffer *, tipidee_response_header const *, uint32_t, tipidee_headers const *, uint32_t, tain const *) ;
extern size_t tipidee_response_header_writemerge_G (buffer *, tipidee_response_header const *, uint32_t, tipidee_headers const *, uint32_t) ;

size_t tipidee_response_file (buffer *, tipidee_rql const *, unsigned int, char const *, struct stat const *, uint64_t, char const *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
size_t tipidee_response_file_G (buffer *, tipidee_rql const *, unsigned int, char const *, struct stat const *, uint64_t, char const *, tipidee_response_header const *, uint32_t, uint32_t) ;

size_t tipidee_response_partial (buffer *, tipidee_rql const *, struct stat const *, uint64_t, uint64_t, char const *, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
size_t tipidee_response_partial_G (buffer *, tipidee_rql const *, struct stat const *, uint64_t, uint64_t, char const *, tipidee_response_header const *, uint32_t, uint32_t) ;

extern size_t tipidee_response_error_nofile (buffer *, tipidee_rql const *, unsigned int, char const *, char const *, tipidee_response_header const *, uint32_t, tipidee_response_header const *, uint32_t, uint32_t, tain const *) ;
extern size_t tipidee_response_error_nofile_G (buffer *, tipidee_rql const *, unsigned int, char const *, char const *, tipidee_response_header const *, uint32_t, tipidee_response_header const *, uint32_t, uint32_t) ;

extern int tipidee_response_header_preparebuiltin (tipidee_response_header *, uint32_t, char const *, size_t) ;

#endif
