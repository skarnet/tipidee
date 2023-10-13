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

struct global_s
{
  tipidee_conf conf ;
  stralloc sa ;
  size_t envlen ;
  size_t cwdlen ;
  size_t indexlen ;
  tain readtto ;
  tain writetto ;
  tain cgitto ;
  char const *defaulthost ;
  char const *indexnames[16] ;
  int p[2] ;
  uint32_t logv ;
  uint32_t maxrqbody ;
  uint32_t maxcgibody ;
  uint16_t defaultport ;
  uint16_t indexn : 4 ;
  uint16_t cont : 2 ;
  uint16_t ssl : 1 ;
} ;
#define GLOBAL_ZERO \
{ \
  .conf = TIPIDEE_CONF_ZERO, \
  .sa = STRALLOC_ZERO, \
  .envlen = 0, \
  .cwdlen = 1, \
  .indexlen = 0, \
  .readtto = TAIN_ZERO, \
  .writetto = TAIN_ZERO, \
  .cgitto = TAIN_ZERO, \
  .defaulthost = "@", \
  .indexnames = { 0 }, \
  .p = { -1, -1 }, \
  .logv = TIPIDEE_LOG_DEFAULT, \
  .maxrqbody = 0, \
  .maxcgibody = 0, \
  .defaultport = 0, \
  .indexn = 0, \
  .cont = 1, \
  .ssl = 0 \
}

extern struct global_s g ;


 /* uid/gid and chroot */

extern void tipideed_harden (unsigned int) ;


 /* Responses */

extern void response_error (tipidee_rql const *, unsigned int, char const *, char const *, uint32_t) ;  /* set bit 0 for Connection: close, bit 1 for preexit */
extern void response_error_and_exit (tipidee_rql const *, unsigned int, char const *, char const *, uint32_t) gccattr_noreturn ;
extern void response_error_and_die (tipidee_rql const *, int e, unsigned int, char const *, char const *, char const *const *, unsigned int, uint32_t) gccattr_noreturn ;

 /*
    preexit is meant to be called before tipidee_log_request(), it won't log an answer line.
    Use for early parsing, when the request isn't even validated yet.
    exit is meant to be called after tipidee_log_request(), it will log an answer line.
    respond can only happen after tipidee_log_request(), it will log an answer line.

    exit will log an informational exit line.
    die will not; there will be a fatal line instead.
    die will log an answer line. There is no "predie", we just use strerr_die instead.
 */

#define preexit_400(r, s) response_error_and_exit(r, 400, "Bad Request", s, 2)
#define exit_400(r, s) response_error_and_exit(r, 400, "Bad Request", s, 0)
extern void exit_405 (tipidee_rql const *, uint32_t) gccattr_noreturn ;  /* set bit 0 for Allow: POST, bit 1 for preexit */
#define preexit_408(r) response_error_and_exit(r, 408, "Request Timeout", "", 2)
#define exit_408(r) response_error_and_exit(r, 408, "Request Timeout", "", 0)
#define preexit_413(r, s) response_error_and_exit(r, 413, "Request Entity Too Large", s, 2)
#define exit_413(r, s) response_error_and_exit(r, 413, "Request Entity Too Large", s, 0)
#define preexit_501(r, s) response_error_and_exit(r, 501, "Not Implemented", s, 2)
#define exit_501(r, s) response_error_and_exit(r, 501, "Not Implemented", s, 0)

#define respond_403(r) response_error(r, 403, "Forbidden", "Missing credentials to access the URI.", 0)
#define respond_404(r) response_error(r, 404, "Not Found", "The request URI was not found.", 0)
#define respond_414(r) response_error(r, 414, "URI Too Long", "The request URI had an oversized component.", 0)
extern void respond_30x (tipidee_rql const *, tipidee_redirection const *) ;
#define respond_504(r) response_error(r, 504, "Gateway Timeout", "The CGI script took too long to answer.", 0)

#define diefx(r, e, status, rsl, text, ...) response_error_and_die(r, e, status, rsl, text, strerr_array(PROG, ": fatal: ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+2, 0)
#define diefusys(r, e, status, rsl, text, ...) response_error_and_die(r, e, status, rsl, text, strerr_array(PROG, ": fatal: ", "unable to ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+3, 1)
#define die500x(r, e, ...) diefx(r, e, 500, "Internal Server Error", "Bad server configuration.", __VA_ARGS__)
#define die500sys(r, e, ...) diefusys(r, e, 500, "Internal Server Error", "System error.", __VA_ARGS__)
#define die502x(r, e, ...) diefx(r, e, 502, "Bad Gateway", "Bad CGI script.", __VA_ARGS__)


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

#endif
