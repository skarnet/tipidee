/* ISC license. */

#ifndef TIPIDEE_METHOD_H
#define TIPIDEE_METHOD_H

typedef enum tipidee_method_e tipidee_method, *tipidee_method_ref ;
enum tipidee_method_e
{
  TIPIDEE_METHOD_GET = 0,
  TIPIDEE_METHOD_HEAD,
  TIPIDEE_METHOD_OPTIONS,
  TIPIDEE_METHOD_POST,
  TIPIDEE_METHOD_PUT,
  TIPIDEE_METHOD_DELETE,
  TIPIDEE_METHOD_TRACE,
  TIPIDEE_METHOD_CONNECT,
  TIPIDEE_METHOD_PRI,
  TIPIDEE_METHOD_UNKNOWN
} ;

typedef struct tipidee_method_conv_s tipidee_method_conv, *tipidee_method_conv_ref ;
struct tipidee_method_conv_s
{
  tipidee_method num ;
  char const *str ;
} ;

extern tipidee_method_conv const *tipidee_method_conv_table ;
extern char const *tipidee_method_tostr (tipidee_method) ;
extern tipidee_method tipidee_method_tonum (char const *) ;

#endif
