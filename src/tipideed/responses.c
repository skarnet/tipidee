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

void response_error_early (tipidee_rql const *rql, unsigned int status, char const *reason, char const *text, uint32_t options)
{
  tain deadline ;
  tipidee_response_error_nofile_G(buffer_1, rql, status, reason, text, g.rhdr, g.rhdrn, options & 1 || !g.cont) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_early_and_exit (tipidee_rql const *rql, unsigned int status, char const *reason, char const *text)
{
  response_error_early(rql, status, reason, text, 1) ;
  log_and_exit(1) ;
}

void response_error (tipidee_rql const *rql, char const *docroot, unsigned int status, uint32_t options)
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
          tipidee_response_file_G(buffer_1, rql, status, dt.reason, &st, tipidee_conf_get_content_type(&g.conf, g.sa.s + pos), g.rhdr, g.rhdrn, options) ;
          tipidee_log_answer(g.logv, rql, status, st.st_size) ;
          send_file(fd, st.st_size, g.sa.s + pos) ;
          fd_close(fd) ;
          return ;
        }
      }
    }
  }

  tipidee_response_error_nofile_G(buffer_1, rql, status, dt.reason, dt.text, g.rhdr, g.rhdrn, options & 1 || !g.cont) ;
  tipidee_log_answer(g.logv, rql, status, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}

void response_error_and_exit (tipidee_rql const *rql, char const *docroot, unsigned int status)
{
  response_error(rql, docroot, status, 1) ;
  log_and_exit(0) ;
}

void response_error_and_die (tipidee_rql const *rql, int e, char const *docroot, unsigned int status, char const *const *v, unsigned int n, uint32_t options)
{
  int serr = errno ;
  response_error(rql, docroot, status, options | 1) ;
  errno = serr ;
  if (options & 1) strerr_dievsys(e, v, n) ;
  else strerr_diev(e, v, n) ;
}

void exit_405_ (tipidee_rql const *rql, uint32_t options)
{
  tain deadline ;
  tipidee_response_status(buffer_1, rql, 405, "Method Not Allowed") ;
  tipidee_response_header_writeall_G(buffer_1, g.rhdr, g.rhdrn, 1) ;
  buffer_putsnoflush(buffer_1, "Allow: GET, HEAD") ;
  if (options & 1) buffer_putsnoflush(buffer_1, ", POST") ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  if (!(options & 2)) tipidee_log_answer(g.logv, rql, 405, 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
  log_and_exit(0) ;
}

void respond_30x (tipidee_rql const *rql, tipidee_redirection const *rd)
{
  static unsigned int const status[4] = { 307, 308, 302, 301 } ;
  static char const *const reason[4] = { "Temporary Redirect", "Permanent Redirect", "Found", "Moved Permanently" } ;
  tain deadline ;
  tipidee_response_status(buffer_1, rql, status[rd->type], reason[rd->type]) ;
  tipidee_response_header_writeall_G(buffer_1, g.rhdr, g.rhdrn, 0) ;
  buffer_putsnoflush(buffer_1, "Content-Length: 0\r\nLocation: ") ;
  buffer_putsnoflush(buffer_1, rd->location) ;
  if (rd->sub) buffer_putsnoflush(buffer_1, rd->sub) ;
  buffer_putnoflush(buffer_1, "\r\n\r\n", 4) ;
  tipidee_log_answer(g.logv, rql, status[rd->type], 0) ;
  tain_add_g(&deadline, &g.writetto) ;
  if (!buffer_timed_flush_g(buffer_1, &deadline))
    strerr_diefu1sys(111, "write to stdout") ;
}
