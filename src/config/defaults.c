/* ISC license. */

#include <stddef.h>

#include "tipidee-config-internal.h"

struct defaults_s
{
  char const *key ;
  char const *value ;
  size_t vlen ;
} ;

#define RECB(k, v, n) { .key = k, .value = v, .vlen = n }
#define REC(k, v) RECB(k, v, sizeof(v))

struct defaults_s const defaults[] =
{
  RECB("G:verbosity", "\0\0\0\001", 4),
  RECB("G:read_timeout", "\0\0\0", 4),
  RECB("G:write_timeout", "\0\0\0", 4),
  RECB("G:cgi_timeout", "\0\0\0", 4),
  RECB("G:max_request_body_length", "\0\0 ", 4),
  RECB("G:max_cgi_body_length", "\0@\0", 4),
  REC("G:index_file", "index.html"),

  REC("T:html", "text/html"),
  REC("T:htm", "text/html"),
  REC("T:txt", "text/plain"),
  REC("T:h", "text/plain"),
  REC("T:c", "text/plain"),
  REC("T:cc", "text/plain"),
  REC("T:cpp", "text/plain"),
  REC("T:java", "text/plain"),
  REC("T:mjs", "text/javascript"),
  REC("T:css", "text/css"),
  REC("T:csv", "text/csv"),
  REC("T:doc", "application/msword"),
  REC("T:docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
  REC("T:js", "application/javascript"),
  REC("T:jpg", "image/jpeg"),
  REC("T:jpeg", "image/jpeg"),
  REC("T:gif", "image/gif"),
  REC("T:png", "image/png"),
  REC("T:bmp", "image/bmp"),
  REC("T:svg", "image/svg+xml"),
  REC("T:tif", "image/tiff"),
  REC("T:tiff", "image/tiff"),
  REC("T:ico", "image/vnd.microsoft.icon"),
  REC("T:au", "audio/basic"),
  REC("T:aac", "audio/aac"),
  REC("T:wav", "audio/wav"),
  REC("T:mid", "audio/midi"),
  REC("T:midi", "audio/midi"),
  REC("T:mp3", "audio/mpeg"),
  REC("T:ogg", "audio/ogg"),
  REC("T:oga", "audio/ogg"),
  REC("T:ogv", "video/ogg"),
  REC("T:avi", "video/x-msvideo"),
  REC("T:wmv", "video/x-ms-wmv"),
  REC("T:qt", "video/quicktime"),
  REC("T:mov", "video/quicktime"),
  REC("T:mpe", "video/mpeg"),
  REC("T:mpeg", "video/mpeg"),
  REC("T:mp4", "video/mp4"),
  REC("T:otf", "font/otf"),
  REC("T:ttf", "font/ttf"),
  REC("T:epub", "application/epub+zip"),
  REC("T:jar", "application/java-archive"),
  REC("T:json", "application/json"),
  REC("T:jsonld", "application/ld+json"),
  REC("T:pdf", "application/pdf"),
  REC("T:ppt", "application/vnd.ms-powerpoint"),
  REC("T:pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"),
  REC("T:odp", "application/vnd.oasis.opendocument.presentation"),
  REC("T:ods", "application/vnd.oasis.opendocument.spreadsheet"),
  REC("T:odt", "application/vnd.oasis.opendocument.text"),
  REC("T:oggx", "application/ogg"),
  REC("T:rar", "application/vnd.rar"),
  REC("T:rtf", "application/rtf"),
  REC("T:sh", "application/x-sh"),
  REC("T:tar", "application/x-tar"),
  REC("T:xhtml", "application/xhtml+xml"),
  REC("T:xls", "application/vnd.ms-excel"),
  REC("T:xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
  REC("T:xml", "application/xml"),
  REC("T:xul", "application/vnd.mozilla.xul+xml"),
  REC("T:zip", "application/zip"),
  REC("T:7z", "application/x-7z-compressed"),

  RECB(0, 0, 0)
} ;

void conf_defaults (void)
{
  for (struct defaults_s const *p = defaults ; p->key ; p++)
  {
    if (!conftree_search(p->key))
    {
      confnode node ;
      confnode_start(&node, p->key, 0, 0) ;
      confnode_add(&node, p->value, p->vlen) ;
      conftree_add(&node) ;
    }
  }
}
