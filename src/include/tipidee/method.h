/* ISC license. */

#ifndef TIPIDEE_METHOD_H
#define TIPIDEE_METHOD_H

typedef enum tipidee_method_e tipidee_method, *tipidee_method_ref ;
enum tipidee_method_e
{
  TIPIDEE_METHOD_CONNECT = 0,
  TIPIDEE_METHOD_DELETE,
  TIPIDEE_METHOD_GET,
  TIPIDEE_METHOD_HEAD,
  TIPIDEE_METHOD_OPTIONS,
  TIPIDEE_METHOD_PATCH,
  TIPIDEE_METHOD_POST,
  TIPIDEE_METHOD_PRI,
  TIPIDEE_METHOD_PUT,
  TIPIDEE_METHOD_TRACE,
  TIPIDEE_METHOD_UNKNOWN
} ;

extern char const *tipidee_method_tostr (tipidee_method) ;
extern tipidee_method tipidee_method_tonum (char const *) ;

#endif
