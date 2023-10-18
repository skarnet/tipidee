/* ISC license. */

#ifndef TIPIDEE_CONF_H
#define TIPIDEE_CONF_H

#include <stddef.h>
#include <stdint.h>

#include <skalibs/uint16.h>
#include <skalibs/cdb.h>

#include <tipidee/uri.h>

#define TIPIDEE_CONF_KEY_MAXLEN 0x1000U

typedef struct tipidee_conf_s tipidee_conf, *tipidee_conf_ref ;
struct tipidee_conf_s
{
  cdb c ;
} ;
#define TIPIDEE_CONF_ZERO { .c = CDB_ZERO }

typedef struct tipidee_redirection_s tipidee_redirection, *tipidee_redirection_ref ;
struct tipidee_redirection_s
{
  char const *location ;
  char const *sub ;
  uint32_t type : 2 ;
} ;
#define TIPIDEE_REDIRECTION_ZERO { .location = 0, .sub = 0, .type = 0 }

extern void tipidee_conf_free (tipidee_conf *) ;
extern int tipidee_conf_init (tipidee_conf *, char const *) ;

extern int tipidee_conf_get (tipidee_conf const *, char const *, cdb_data *) ;
extern char const *tipidee_conf_get_string (tipidee_conf const *, char const *) ;
extern int tipidee_conf_get_uint32 (tipidee_conf const *, char const *, uint32_t *) ;
extern unsigned int tipidee_conf_get_argv (tipidee_conf const *, char const *, char const **, unsigned int, size_t *) ;

extern char const *tipidee_conf_get_docroot (tipidee_conf const *, tipidee_uri const *, uint16_t) ;
extern int tipidee_conf_get_redirection (tipidee_conf const *, char const *, size_t, char const *, tipidee_redirection *) ;
extern char const *tipidee_conf_get_content_type (tipidee_conf const *, char const *) ;
extern char const *tipidee_conf_get_errorfile (tipidee_conf const *, char const *, unsigned int) ;

#endif
