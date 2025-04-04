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
  tipidee_response_header const *rhdr ;
  int p[2] ;
  uint32_t tarpit ;
  uint32_t rhdrn ;
  uint32_t logv ;
  uint32_t maxrqbody ;
  uint32_t maxcgibody ;
  uint16_t defaultport ;
  uint16_t indexn : 4 ;
  uint16_t cont : 2 ;
  uint16_t ssl : 1 ;
  uint16_t xiscgi : 1 ;
  uint8_t flagnoxlate : 1 ;
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
  .tarpit = 0, \
  .rhdr = 0, \
  .p = { -1, -1 }, \
  .rhdrn = 0, \
  .logv = TIPIDEE_LOG_DEFAULT, \
  .maxrqbody = 0, \
  .maxcgibody = 0, \
  .defaultport = 0, \
  .indexn = 0, \
  .cont = 1, \
  .ssl = 0, \
  .xiscgi = 0, \
  .flagnoxlate = 0 \
}

extern struct global_s g ;


 /* uid/gid and chroot */

extern void tipideed_harden (unsigned int) ;


 /* errors */

#define response_error_early(rql, status, reason, text, options) response_error_early_plus(rql, status, reason, text, 0, 0, options)
extern void response_error_early_plus (tipidee_rql const *, unsigned int, char const *, char const *, tipidee_response_header const *, uint32_t, uint32_t) ;
#define response_error_early_and_exit(rql, status, reason, text) response_error_early_plus_and_exit(rql, status, reason, (text), 0, 0)
extern void response_error_early_plus_and_exit (tipidee_rql const *, unsigned int, char const *, char const *, tipidee_response_header const *, uint32_t) gccattr_noreturn ;
extern void eexit_405 (tipidee_rql const *) gccattr_noreturn ;

#define eexit_400(r, s) response_error_early_and_exit(r, 400, "Bad Request", s)
#define eexit_408(r) response_error_early_and_exit((r), 408, "Request Timeout", 0)
#define eexit_413(r, s) response_error_early_and_exit(r, 413, "Request Entity Too Large", s)
#define eexit_501(r, s) response_error_early_and_exit(r, 501, "Not Implemented", s)

#define response_error(rql, docroot, status, options) response_error_plus(rql, docroot, status, 0, 0, options)
extern void response_error_plus (tipidee_rql const *, char const *, unsigned int, tipidee_response_header const *, uint32_t, uint32_t) ;  /* set bit 0 for Connection: close */
#define response_error_and_exit(rql, docroot, status) response_error_plus_and_exit(rql, docroot, (status), 0, 0)
extern void response_error_plus_and_exit (tipidee_rql const *, char const *, unsigned int, tipidee_response_header const *, uint32_t) gccattr_noreturn ;
#define response_error_and_die(rql, e, docroot, status, v, n, options) response_error_plus_and_die(rql, e, docroot, status, 0, 0, v, n, options)
extern void response_error_plus_and_die (tipidee_rql const *, int, char const *, unsigned int, tipidee_response_header const *, uint32_t, char const *const *, unsigned int, uint32_t) gccattr_noreturn ;  /* set bit 0 for diesys */

extern void exit_405 (tipidee_rql const *, char const *, uint32_t) gccattr_noreturn ;
#define exit_400(r, d) response_error_and_exit(r, (d), 400)
#define exit_408(r, d) response_error_and_exit(r, (d), 408)
#define exit_413(r, d) response_error_and_exit(r, (d), 413)
#define exit_501(r, d) response_error_and_exit(r, (d), 501)

extern void respond_416 (tipidee_rql const *, char const *, uint64_t) ;

#define respond_403(r, d) response_error(r, (d), 403, 0)
#define respond_404(r, d) response_error(r, (d), 404, 0)
#define respond_414(r, d) response_error(r, (d), 414, 0)
#define respond_504(r, d) response_error(r, (d), 504, 0)

#define diefx(r, e, d, status, ...) response_error_and_die(r, e, d, status, strerr_array(PROG, ": fatal: ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+2, 0)
#define diefusys(r, e, d, status, ...) response_error_and_die(r, e, d, status, strerr_array(PROG, ": fatal: ", "unable to ", __VA_ARGS__), sizeof(strerr_array(__VA_ARGS__))/sizeof(char const *)+3, 1)
#define die500x(r, e, d, ...) diefx(r, e, d, 500, __VA_ARGS__)
#define die500sys(r, e, d, ...) diefusys(r, e, d, 500, __VA_ARGS__)
#define die502x(r, e, d, ...) diefx(r, e, d, 502, __VA_ARGS__)


 /* redirection */

extern void respond_30x (tipidee_rql const *, tipidee_redirection const *) ;


 /* trace */

extern int respond_trace (tipidee_rql const *, tipidee_headers const *) ;


 /* options */

extern int respond_options (tipidee_rql const *, uint32_t) ;


 /* send_file */

extern void init_splice_pipe (void) ;
extern void send_file_range (int, uint64_t, uint64_t, char const *) ;
#define send_file(fd, n, fn) send_file_range(fd, 0, n, fn)


 /* stream */

extern void stream_fixed (int, uint64_t, char const *) ;
extern void stream_autochunk (buffer *, char const *) ;
extern void stream_infinite (int, char const *) ;


 /* regular */

extern int respond_regular (tipidee_rql const *, char const *, char const *, struct stat const *, tipidee_resattr const *) ;
extern int respond_partial (tipidee_rql const *, char const *, char const *, struct stat const *, uint64_t, uint64_t, tipidee_resattr const *) ;
extern int respond_304 (tipidee_rql const *, char const *, struct stat const *) ;


 /* cgi */

extern int respond_cgi (tipidee_rql *, char const *, char const *, size_t, char const *, char *, tipidee_headers const *, tipidee_resattr const *, char const *, size_t) ;


 /* util */

extern size_t translate_path (char const *) ;


 /* main */

extern void log_and_exit (int) gccattr_noreturn ;

#endif
