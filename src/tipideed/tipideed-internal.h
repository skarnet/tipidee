/* ISC license. */

#ifndef TIPIDEED_INTERNAL_H
#define TIPIDEED_INTERNAL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#include <skalibs/gccattributes.h>
#include <skalibs/uint64.h>
#include <skalibs/stralloc.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>

#include <tipidee/tipidee.h>

#define URI_BUFSIZE 4096
#define HDR_BUFSIZE 8192

typedef struct tipidee_resattr_s tipidee_resattr, *tipidee_resattr_ref ;
struct tipidee_resattr_s
{
  char const *content_type ;
  uint32_t iscgi : 1 ;
  uint32_t isnph : 1 ;
} ;
#define TIPIDEE_RESATTR_ZERO { .content_type = 0, .iscgi = 0, .isnph = 0 }

struct global_s
{
  tipidee_conf conf ;
  stralloc sa ;
  size_t envlen ;
  size_t localip ;
  size_t localport ;
  size_t localhost ;
  size_t remoteip ;
  size_t remoteport ;
  size_t remotehost ;
  size_t cwdlen ;
  size_t indexlen ;
  tain readtto ;
  tain writetto ;
  tain cgitto ;
  char const *defaulthost ;
  char const *indexnames[16] ;
  int p[2] ;
  uint32_t maxrqbody ;
  uint32_t maxcgibody ;
  uint16_t defaultport ;
  uint16_t indexn : 4 ;
  uint16_t verbosity : 3 ;
  uint16_t cont : 2 ;
  uint16_t ssl : 1 ;
} ;
#define GLOBAL_ZERO \
{ \
  .conf = TIPIDEE_CONF_ZERO, \
  .sa = STRALLOC_ZERO, \
  .envlen = 0, \
  .localip = 0, \
  .localport = 0, \
  .localhost = 0, \
  .remoteip = 0, \
  .remoteport = 0, \
  .remotehost = 0, \
  .cwdlen = 1, \
  .indexlen = 0, \
  .readtto = TAIN_ZERO, \
  .writetto = TAIN_ZERO, \
  .cgitto = TAIN_ZERO, \
  .defaulthost = "@", \
  .indexnames = { 0 }, \
  .p = { -1, -1 }, \
  .maxrqbody = 0, \
  .maxcgibody = 0, \
  .defaultport = 0, \
  .indexn = 0, \
  .verbosity = 1, \
  .cont = 1, \
  .ssl = 0 \
}

extern struct global_s g ;


 /* uid/gid and chroot */

extern void tipideed_harden (unsigned int) ;


 /* Responses */

extern void response_error (tipidee_rql const *, char const *, char const *, int) ;
extern void response_error_and_exit (tipidee_rql const *, char const *, char const *) gccattr_noreturn ;
extern void response_error_and_die (tipidee_rql const *, int e, char const *, char const *, char const *const *, unsigned int, int) gccattr_noreturn ;

#define exit_400(r, s) response_error_and_exit(r, "400 Bad Request", s)
extern void exit_405 (tipidee_rql const *, uint32_t) gccattr_noreturn ;
#define exit_408(r) response_error_and_exit(r, "408 Request Timeout", "")
#define exit_413(r, s) response_error_and_exit(r, "413 Request Entity Too Large", s)
#define exit_501(r, s) response_error_and_exit(r, "501 Not Implemented", s)

#define respond_403(r) response_error(r, "403 Forbidden", "Missing credentials to access the URI.", 0)
#define respond_404(r) response_error(r, "404 Not Found", "The request URI was not found.", 0)
#define respond_414(r) response_error(r, "414 URI Too Long", "The request URI had an oversized component.", 0)
extern void respond_30x (tipidee_rql const *, tipidee_redirection const *) ;
#define respond_504(r) response_error(r, "504 Gateway Timeout", "The CGI script took too long to answer.", 0)

#define diefx(r, e, rsl, text, ...) response_error_and_die(r, e, rsl, text, strerr_array(PROG, ": fatal: ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+2, 0)
#define diefusys(r, e, rsl, text, ...) response_error_and_die(r, e, rsl, text, strerr_array(PROG, ": fatal: ", "unable to ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+3, 1)
#define die500x(r, e, ...) diefx(r, e, "500 Internal Server Error", "Bad server configuration.", __VA_ARGS__)
#define die500sys(r, e, ...) diefusys(r, e, "500 Internal Server Error", "System error.", __VA_ARGS__)
#define die502x(r, e, ...) diefx(r, e, "502 Bad Gateway", "Bad CGI script.", __VA_ARGS__)

 /* Trace */

extern int respond_trace (char const *, tipidee_rql const *, tipidee_headers const *) ;


 /* Options */

extern int respond_options (tipidee_rql const *, uint32_t) ;


 /* send_file */

extern void init_splice_pipe (void) ;
extern void send_file (int, uint64_t, char const *) ;


 /* regular */

extern int respond_regular (tipidee_rql const *, char const *, struct stat const *, tipidee_resattr const *) ;
extern int respond_304 (tipidee_rql const *, char const *, struct stat const *) ;


 /* cgi */

extern int respond_cgi (tipidee_rql *, char const *, size_t, char const *, char *, tipidee_headers const *, tipidee_resattr const *, char const *, size_t) ;


 /* log */

extern void log_start (void) ;
extern void log_and_exit (int) gccattr_noreturn ;
extern void log_request (tipidee_rql const *) ;
extern void log_regular (char const *, char const *, int, char const *) ;
extern void log_response (char const *, char const *) ;
extern void log_nph (char const *const *, char const *const *) ;
extern void log_cgi (char const *const *, char const *const *) ;

#endif
