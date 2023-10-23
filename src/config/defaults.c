/* ISC license. */

#include <stddef.h>

#include <tipidee/log.h>
#include "tipidee-config-internal.h"

struct defaults_s
{
  char const *key ;
  char const *value ;
  size_t vlen ;
} ;

#define REC(k, v, n) { .key = (k), .value = (v), .vlen = (n) }
#define RECS(k, v) REC(k, v, sizeof(v))
#define RECU32(k, u) { .key = (k), .value = (char const [4]){ (u) >> 24 & 0xffu, (u) >> 16 & 0xffu, (u) >> 8 & 0xffu, (u) & 0xffu }, .vlen = 4 }

static struct defaults_s const defaults[] =
{
  RECU32("G:read_timeout", 0),
  RECU32("G:write_timeout", 0),
  RECU32("G:cgi_timeout", 0),
  RECU32("G:max_request_body_length", 8192),
  RECU32("G:max_cgi_body_length", 4194304),
  RECS("G:index_file", "index.html"),
  RECU32("G:logv", TIPIDEE_LOG_DEFAULT),

  RECS("T:html", "text/html"),
  RECS("T:htm", "text/html"),
  RECS("T:txt", "text/plain"),
  RECS("T:h", "text/plain"),
  RECS("T:c", "text/plain"),
  RECS("T:cc", "text/plain"),
  RECS("T:cpp", "text/plain"),
  RECS("T:ass", "text/plain"),
  RECS("T:java", "text/plain"),
  RECS("T:mjs", "text/javascript"),
  RECS("T:css", "text/css"),
  RECS("T:csv", "text/csv"),
  RECS("T:sub", "text/vnd.dvb.subtitle"),
  RECS("T:doc", "application/msword"),
  RECS("T:docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
  RECS("T:js", "application/javascript"),
  RECS("T:jpg", "image/jpeg"),
  RECS("T:jpeg", "image/jpeg"),
  RECS("T:gif", "image/gif"),
  RECS("T:png", "image/png"),
  RECS("T:bmp", "image/bmp"),
  RECS("T:svg", "image/svg+xml"),
  RECS("T:tif", "image/tiff"),
  RECS("T:tiff", "image/tiff"),
  RECS("T:ico", "image/vnd.microsoft.icon"),
  RECS("T:au", "audio/basic"),
  RECS("T:aac", "audio/aac"),
  RECS("T:wav", "audio/wav"),
  RECS("T:mid", "audio/midi"),
  RECS("T:midi", "audio/midi"),
  RECS("T:mp3", "audio/mpeg"),
  RECS("T:ogg", "audio/ogg"),
  RECS("T:oga", "audio/ogg"),
  RECS("T:ogv", "video/ogg"),
  RECS("T:avi", "video/x-msvideo"),
  RECS("T:wmv", "video/x-ms-wmv"),
  RECS("T:qt", "video/quicktime"),
  RECS("T:mov", "video/quicktime"),
  RECS("T:mpe", "video/mpeg"),
  RECS("T:mpeg", "video/mpeg"),
  RECS("T:mp4", "video/mp4"),
  RECS("T:mkv", "video/x-matroska"),
  RECS("T:otf", "font/otf"),
  RECS("T:ttf", "font/ttf"),
  RECS("T:epub", "application/epub+zip"),
  RECS("T:jar", "application/java-archive"),
  RECS("T:json", "application/json"),
  RECS("T:jsonld", "application/ld+json"),
  RECS("T:pdf", "application/pdf"),
  RECS("T:ppt", "application/vnd.ms-powerpoint"),
  RECS("T:pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"),
  RECS("T:odp", "application/vnd.oasis.opendocument.presentation"),
  RECS("T:ods", "application/vnd.oasis.opendocument.spreadsheet"),
  RECS("T:odt", "application/vnd.oasis.opendocument.text"),
  RECS("T:oggx", "application/ogg"),
  RECS("T:rar", "application/vnd.rar"),
  RECS("T:rtf", "application/rtf"),
  RECS("T:sh", "application/x-sh"),
  RECS("T:srt", "application/x-subrip"),
  RECS("T:tar", "application/x-tar"),
  RECS("T:xhtml", "application/xhtml+xml"),
  RECS("T:xls", "application/vnd.ms-excel"),
  RECS("T:xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
  RECS("T:xml", "application/xml"),
  RECS("T:xul", "application/vnd.mozilla.xul+xml"),
  RECS("T:zip", "application/zip"),
  RECS("T:7z", "application/x-7z-compressed"),

  REC(0, 0, 0)
} ;

void conf_defaults (void)
{
  for (struct defaults_s const *p = defaults ; p->key ; p++)
  {
    if (!conftree_search(p->key))
    {
      node node ;
      confnode_start(&node, p->key, 0, 0) ;
      confnode_add(&node, p->value, p->vlen) ;
      conftree_add(&node) ;
    }
  }
}
