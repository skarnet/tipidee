/* ISC license. */

#ifndef TIPIDEE_REDIRECTION_H
#define TIPIDEE_REDIRECTION_H

#include <stdint.h>

enum tipidee_redir_type_e
{
  TIPIDEE_REDIR_NONE,
  TIPIDEE_REDIR_REDIRECT,
  TIPIDEE_REDIR_RPROXY,
  TIPIDEE_REDIR_FASTCGI
} ;


 /* for redirect */
#define TIPIDEE_REDIR_307 0x0
#define TIPIDEE_REDIR_308 0x1
#define TIPIDEE_REDIR_302 0x2
#define TIPIDEE_REDIR_301 0x3

 /* for rproxy or fastcgi */
#define TIPIDEE_REDIR_ISINET 0x1
#define TIPIDEE_REDIR_ISV6 0x2

typedef struct tipidee_redirection_s tipidee_redirection, *tipidee_redirection_ref ;
struct tipidee_redirection_s
{
  char const *sub ;
  char const *addr ;
  uint16_t port ;
  uint8_t type : 4 ;
  uint8_t flags : 4 ;
} ;
#define TIPIDEE_REDIRECTION_ZERO { .sub = 0, .addr = 0, .port = 0, .type = TIPIDEE_REDIR_NONE, .flags = 0 }

#endif
