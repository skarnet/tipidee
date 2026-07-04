/* ISC license. */

#include <stdint.h>

#include <skalibs/uint64.h>
#include <skalibs/types.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>

#include <tipidee/tipidee.h>
#include "tipideed-internal.h"

int local_redirect (tipidee_rql *rql, char const *docroot, char const *loc, char *uribuf, char const *what, char const *cginame)
{
  size_t n ;
  size_t hostlen = strlen(rql->uri.host) ;
  uint16_t port = rql->uri.port ;
  uint8_t ishttps = rql->uri.https ;
  char hosttmp[hostlen + 1] ;
  memcpy(hosttmp, rql->uri.host, hostlen + 1) ;
  n = tipidee_uri_parse(uribuf, URI_BUFSIZE, loc, &rql->uri) ;
  if (!n || n + hostlen + 1 > URI_BUFSIZE)
    die502x(rql, 2, docroot, what, " ", cginame, " returned an invalid ", "Location", " value", " for local redirection") ;
  memcpy(uribuf + n, hosttmp, hostlen + 1) ;
  rql->uri.host = uribuf + n ;
  rql->uri.port = port ;
  rql->uri.https = ishttps ;
  tipidee_log_debug(g.logv, what, " ", cginame, ": local redirect to ", rql->uri.path) ;
  return 1 ;
}

int process_cgi_headers (tipidee_rql const *rql, tipidee_headers *hdr, char const *docroot, char const *what, char const *app, int autochunk, uint64_t *cl)
{
  char const *reason = "OK" ;
  char const *location ;
  char const *contentlength ;
  char const *statusfield ;
  unsigned int status = 0 ;

  statusfield = tipidee_headers_search(hdr, "Status") ;
  location = tipidee_headers_search(hdr, "Location") ;
  contentlength = tipidee_headers_search(hdr, "Content-Length") ;
  if (contentlength)
  {
    if (!uint640_scan(contentlength, cl))
      die502x(rql, 2, docroot, what, " ", app, " returned an invalid ", "Content-Length", " header") ;
  }
  if (statusfield)
  {
    size_t m = uint_scan(statusfield, &status) ;
    if (!m || (statusfield[m] && statusfield[m] != ' '))
      die502x(rql, 2, docroot, what, " ", app, " returned an invalid ", "Status", " header") ;
    if (statusfield[m]) reason = statusfield + m + 1 ;
    else
    {
      tipidee_defaulttext dt ;
      reason = tipidee_util_defaulttext(status, &dt) ? dt.reason : "" ;
    }
    if (!location && (status == 301 || status == 302 || status == 307 || status == 308))
      die502x(rql, 2, docroot, what, " ", app, " returned a redirection status code without a ", "Location", " header") ;
    if (status < 100 || status > 999)
      die502x(rql, 2, docroot, what, " ", app, " returned an invalid ", "Status", " value") ;
  }
  if (location)
  {
    if (!location[0]) die502x(rql, 2, docroot, what, " ", app, " returned an invalid ", "Location", " header") ;
    if (location[0] == '/' && location[1] != '/') return 2 ;
    if (*cl)
    {
      if (!status)
        die502x(rql, 2, docroot, what, " ", app, " didn't output a ", "Status", " header", " for a client redirect response with document") ;
      if (status < 300 || status > 399)
        die502x(rql, 2, docroot, what, " ", app, " returned an invalid ", "Status", " value", " for a client redirect response with document") ;
    }
    else
    {
      for (uint32_t i = 0 ; i < hdr->n ; i++)
      {
        char const *key = hdr->buf + hdr->list[i].left ;
        if (!strcasecmp(key, "Location") || !strcasecmp(key, "Status")) continue ;
        if (str_start(key, "X-CGI-")) continue ;
        die502x(rql, 2, docroot, what, " ", app, " returned extra headers", " for a client redirect response without document") ;
      }
      if (!status)
      {
        status = 302 ;
        reason = "Found" ;
      }
    }
  }
  else
  {
    if (!status) status = 200 ;
    if (status != 304 && (status < 400 || status > 599) && !tipidee_headers_search(hdr, "Content-Type"))
      die502x(rql, 2, docroot, what, " ", app, " didn't output a ", "Content-Type", " header") ;
  }

  tipidee_response_status(buffer_1, rql, status, reason) ;
  tipidee_response_header_writemerge_G(buffer_1, g.rhdr, g.rhdrn, hdr, !g.cont) ;

  if (contentlength)
  {
    char fmt[UINT64_FMT] ;
    fmt[uint64_fmt(fmt, *cl)] = 0 ;
    buffer_putsnoflush(buffer_1, "Content-Length: ") ;
    buffer_putsnoflush(buffer_1, fmt) ;
    buffer_putnoflush(buffer_1, "\r\n", 2) ;
  }
  else if (autochunk)
    buffer_putsnoflush(buffer_1, "Transfer-Encoding: chunked\r\n") ;

  buffer_putnoflush(buffer_1, "\r\n", 2) ;
  tipidee_log_answer(g.logv, rql, status, *cl) ;
  return !!contentlength ;
}
