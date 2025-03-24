/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <skalibs/stat.h>
#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/tai.h>
#include <skalibs/djbunix.h>
#include <skalibs/unix-timed.h>

#include <tipidee/log.h>
#include <tipidee/util.h>
#include <tipidee/response.h>

#include "tipideed-internal.h"

void response_error_early_plus (tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, tipidee_response_header const *v, uint32_t n, uint32_t options)
{
  tain deadline ;
  tipidee_response_error_nofile_G(buffer_1, rql, status, reason, text, g.rhdr, g.rhdrn, v, n, options & 1 || !g.cont) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_early_plus_and_exit (tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, tipidee_response_header const *v, uint32_t n)
{
  response_error_early_plus(rql, status, reason, text, v, n, 1) ;
  log_and_exit(1) ;
}

void eexit_405 (tipidee_rql const *rql)
{
  static tipidee_response_header const allowpost = { .key = "Allow", .value = "GET, HEAD, POST, PUT, DELETE, PATCH", .options = 0 } ;
  response_error_early_plus_and_exit(rql, 405, "Method Not Allowed", "Invalid method for the resource.", &allowpost, 1) ;
}

void response_error_plus (tipidee_rql const *rql, char const *docroot, unsigned int status, tipidee_response_header const *plus, uint32_t plusn, uint32_t options)
{
  tain deadline ;
  tipidee_defaulttext dt ;
  char const *file = 0 ;
  size_t pos = translate_path(docroot) ;

  if (pos) file = tipidee_conf_get_errorfile(&g.conf, g.sa.s + pos , status) ;
  else if (errno == EPERM)
    strerr_dief4x(102, "layout error: ", "docroot ", docroot, " points outside of the server's root") ;
  else if (errno != ENOENT)
    strerr_diefu2sys(111, "translate_path ", docroot) ;

  if (!tipidee_util_defaulttext(status, &dt))
  {
    char fmt[UINT_FMT] ;
    fmt[uint_fmt(fmt, status)] = 0 ;
    strerr_dief2x(101, "can't happen: unknown response code ", fmt) ;
  }

  if (file)
  {
    pos = translate_path(file) ;
    if (!pos)
    {
      if (errno == EPERM)
        strerr_warnw4x("layout error: ", "custom response file ", file, " points outside of the server's root") ;
      else
        strerr_warnwu3sys("translate_path ", "custom response file ", file) ;
    }
    else
    {
      int fd = open_read(g.sa.s + pos) ;
      if (fd == -1) strerr_warnwu3sys("open ", "custom response file ", file) ;
      else
      {
        struct stat st ;
        if (fstat(fd, &st) == -1)
        {
          fd_close(fd) ;
          strerr_warnwu3sys("stat ", "custom response file ", file) ;
        }
        else if (!S_ISREG(st.st_mode))
        {
          fd_close(fd) ;
          strerr_warnw3x("custom response file ", file, " is not a regular file") ;
        }
        else
        {
          tipidee_response_file_G(buffer_1, rql, status, dt.reason, &st, st.st_size, tipidee_conf_get_content_type(&g.conf, g.sa.s + pos), g.rhdr, g.rhdrn, options) ;
          tipidee_response_header_write(buffer_1, plus, plusn) ;
          tipidee_response_header_end(buffer_1) ;
          tipidee_log_answer(g.logv, rql, status, st.st_size) ;
          send_file(fd, st.st_size, g.sa.s + pos) ;
          fd_close(fd) ;
          return ;
        }
      }
    }
  }

  tipidee_response_error_nofile_G(buffer_1, rql, status, dt.reason, dt.text, g.rhdr, g.rhdrn, plus, plusn, options & 1 || !g.cont) ;
  tipidee_log_answer(g.logv, rql, status, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_plus_and_exit (tipidee_rql const *rql, char const *docroot, unsigned int status, tipidee_response_header const *plus, uint32_t plusn)
{
  response_error_plus(rql, docroot, status, plus, plusn, 1) ;
  log_and_exit(0) ;
}

void response_error_plus_and_die (tipidee_rql const *rql, int e, char const *docroot, unsigned int status, tipidee_response_header const *plus, uint32_t plusn, char const *const *v, unsigned int n, uint32_t options)
{
  int serr = errno ;
  response_error_plus(rql, docroot, status, plus, plusn, options | 1) ;
  errno = serr ;
  if (options & 1) strerr_dievsys(e, v, n) ;
  else strerr_diev(e, v, n) ;
}

void exit_405 (tipidee_rql const *rql, char const *docroot, uint32_t options)
{
  tipidee_response_header hd = { .key = "Allow", .value = options & 1 ? "GET, HEAD, POST, PUT, DELETE, PATCH" : "GET, HEAD", .options = 0 } ;
  response_error_plus_and_exit(rql, docroot, 405, &hd, 1) ;
}

void respond_416 (tipidee_rql const *rql, char const *docroot, uint64_t size)
{
  char buf[8 + UINT64_FMT] = "bytes */" ;
  tipidee_response_header cr = { .key = "Content-Range", .value = buf, .options = 0 } ;
  buf[8 + uint64_fmt(buf + 8, size)] = 0 ;
  response_error_plus(rql, docroot, 416, &cr, 1, 0) ;
}
