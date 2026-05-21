/* ISC license. */

#ifndef TIPIDEE_FCGI_H
#define TIPIDEE_FCGI_H

#include <stdint.h>

typedef struct fcgi_header_s fcgi_header, *fcgi_header_ref ;
struct fcgi_header_s
{
  uint8_t version ;
  uint8_t type ;
  uint16_t requestid ;
  uint16_t len ;
  uint8_t padlen ;
  uint8_t reserved ;
} ;

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
  FCGI_MAXTYPE
} ;

#define FCGI_NULL_REQUEST_ID 0

extern void tipidee_fcgi_header_pack (char *, fcgi_header const *) ;
extern void tipidee_fcgi_header_unpack (char const *, fcgi_header *) ;


typedef struct fcgi_beginrequest_body_s fcgi_beginrequest_body, *fcgi_beginrequest_body_ref ;
struct fcgi_beginrequest_body_s
{
  uint16_t role ;
  uint8_t flags;
  unsigned char reserved[5] ;
} ;

extern void tipidee_fcgi_beginrequest_body_pack (char *, fcgi_beginrequest_body const *) ;
extern void tipidee_fcgi_beginrequest_body_unpack (char const *, fcgi_beginrequest_body *) ;


typedef struct fcgi_beginrequest_record_s fcgi_beginrequest_record, *fcgi_beginrequest_record_ref ;
struct fcgi_beginrequest_record_s
{
  fcgi_header header ;
  fcgi_beginrequest_body body ;
} ;

#define FCGI_KEEP_CONN  1

#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

extern void tipidee_fcgi_beginrequest_record_pack (char *, fcgi_beginrequest_record const *) ;
extern void tipidee_fcgi_beginrequest_record_unpack (char const *, fcgi_beginrequest_record *) ;


typedef struct fcgi_endrequest_body_s fcgi_endrequest_body, *fcgi_endrequest_body_ref ;
struct fcgi_endrequest_body_s
{
  uint32_t appstatus ;
  uint8_t protostatus ;
  uint8_t reserved[3] ;
} ;

extern void tipidee_fcgi_endrequest_body_pack (char *, fcgi_endrequest_body const *) ;
extern void tipidee_fcgi_endrequest_body_unpack (char const *, fcgi_endrequest_body *) ;


typedef struct fcgi_endrequest_record_s fcgi_endrequest_record, *fcgi_endrequest_record_ref ;
struct fcgi_endrequest_record_s
{
  fcgi_header header ;
  fcgi_endrequest_body body ;
} ;

#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"

extern void tipidee_fcgi_endrequest_record_pack (char *, fcgi_endrequest_record const *) ;
extern void tipidee_fcgi_endrequest_record_unpack (char const *, fcgi_endrequest_record *) ;


typedef struct fcgi_unknowntype_body_s fcgi_unknowntype_body, *fcgi_unknowntype_body_ref ;
struct fcgi_unknowntype_body_s
{
  uint8_t type ;
  uint8_t reserved[7] ;
} ;

extern void tipidee_fcgi_unknowntype_body_pack (char *, fcgi_unknowntype_body const *) ;
extern void tipidee_fcgi_unknowntype_body_unpack (char const *, fcgi_unknowntype_body *) ;


typedef struct fcgi_unknowntype_record_s fcgi_unknowntype_record, *fcgi_unknowntype_record_ref ;
struct fcgi_unknowntype_record_s
{
  fcgi_header header ;
  fcgi_unknowntype_body body ;
} ;

extern void tipidee_fcgi_unknowntype_record_pack (char *, fcgi_unknowntype_record const *) ;
extern void tipidee_fcgi_unknowntype_record_unpack (char const *, fcgi_unknowntype_record *) ;

#endif
