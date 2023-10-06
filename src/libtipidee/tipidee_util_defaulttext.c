/* ISC license. */

#include <stdlib.h>

#include <tipidee/util.h>

struct blah_s
{
  unsigned int status ;
  char const *reason ;
  char const *text ;
} ;

static struct blah_s const data[] =
{
  { .status = 200, .reason = "OK", .text = 0 },
  { .status = 206, .reason = "Partial Content", .text = 0 },
  { .status = 301, .reason = "Moved Permanently", .text = 0 },
  { .status = 302, .reason = "Found", .text = 0 },
  { .status = 304, .reason = "Not Modified", .text = 0 },
  { .status = 307, .reason = "Temporary Redirect", .text = 0 },
  { .status = 308, .reason = "Permanent Redirect", .text = 0 },
  { .status = 400, .reason = "Bad Request", .text = "Bad HTTP request." },
  { .status = 401, .reason = "Unauthorized", .text = "You need to be authenticated to access this resource." },
  { .status = 403, .reason = "Forbidden", .text = "Missing credentials to access the resource." },
  { .status = 404, .reason = "Not Found", .text = "The requested resource was not found." },
  { .status = 405, .reason = "Method Not Allowed", .text = "This method is not allowed on this resource." },
  { .status = 408, .reason = "Request Timeout", .text = "The client took too long to formulate its request." },
  { .status = 412, .reason = "Precondition Failed", .text = 0 },
  { .status = 413, .reason = "Content Too Large", .text = "Too much data in the request body." },
  { .status = 414, .reason = "URI Too Long", .text = "The request URI had an oversized component." },
  { .status = 416, .reason = "Range Not Satisfiable", .text = 0 },
  { .status = 500, .reason = "Internal Server Error", .text = "There was a server-side issue while processing your request. Sorry." },
  { .status = 501, .reason = "Not Implemented", .text = "The server does not implement this method." },
  { .status = 502, .reason = "Bad Gateway", .text = "There was an issue with the backend while processing your request. Sorry." },
  { .status = 504, .reason = "Gateway Timeout", .text = "The backend took too long to answer. Sorry." },
  { .status = 505, .reason = "HTTP Version Not Supported", .text = "The server does not implement this version of the protocol." },
} ;

static int blah_cmp (void const *key, void const *el)
{
  unsigned int a = *(unsigned int const *)key ;
  unsigned int b = ((struct blah_s *)el)->status ;
  return a < b ? -1 : a > b ;
}

int tipidee_util_defaulttext (unsigned int status, tipidee_defaulttext *dt)
{
  struct blah_s const *p = bsearch(
    &status,
    data,
    sizeof(data) / sizeof(struct blah_s),
    sizeof(struct blah_s),
    &blah_cmp) ;
  if (!p) return 0 ;
  dt->reason = p->reason ;
  dt->text = p->text ;
  return 1 ;
}
