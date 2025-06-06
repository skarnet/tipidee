/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/skamisc.h>

#include <tipidee/log.h>
#include <tipidee/config.h>
#include "tipidee-config-internal.h"

#define dietoobig() strerr_diefu1sys(100, "read configuration")

typedef struct mdt_s mdt, *mdt_ref ;
struct mdt_s
{
  size_t filepos ;
  uint32_t line ;
  char linefmt[UINT32_FMT] ;
} ;
#define MDT_ZERO { .filepos = 0, .line = 0, .linefmt = "0" }

struct namevalue_s
{
  char const *name ;
  uint32_t value ;
} ;

enum directivevalue_e
{
  T_BANG,
  T_GLOBAL,
  T_INDEXFILE,
  T_LOG,
  T_CONTENTTYPE,
  T_CUSTOMHEADER,
  T_DOMAIN,
  T_NPHPREFIX,
  T_REDIRECT,
  T_NOREDIRECT,
  T_CGI,
  T_NONCGI,
  T_NPH,
  T_NONNPH,
  T_BASICAUTH,
  T_NOAUTH,
  T_AUTOCHUNK,
  T_NOAUTOCHUNK,
  T_FILETYPE,
  T_CUSTOMRESPONSE
} ;

enum customheaderoptionvalue_e
{
  CHO_ADD,
  CHO_ALWAYS,
  CHO_REMOVE,
  CHO_NEVER
} ;

static void conftree_checkunique (char const *key, mdt const *md)
{
  node const *node = conftree_search(key) ;
  if (node)
  {
    char fmt[UINT32_FMT] ;
    fmt[uint32_fmt(fmt, node->line)] = 0 ;
    strerr_diefn(1, 12, "duplicate ", "key ", key, " in file ", g.storage.s + md->filepos, " line ", md->linefmt, ", previously defined", " in file ", g.storage.s + node->filepos, " line ", fmt) ;
  }
}

static void headers_checkunique (char const *key, mdt const *md)
{
  node const *node = headers_search(key) ;
  if (node)
  {
    char fmt[UINT32_FMT] ;
    fmt[uint32_fmt(fmt, node->line)] = 0 ;
    strerr_diefn(1, 12, "duplicate ", "header ", key, " in file ", g.storage.s + md->filepos, " line ", md->linefmt, ", previously defined", " in file ", g.storage.s + node->filepos, " line ", fmt) ;
  }
}

static void add_unique (char const *key, char const *value, size_t valuelen, mdt const *md)
{
  node node ;
  conftree_checkunique(key, md) ;
  confnode_start(&node, key, md->filepos, md->line) ;
  confnode_add(&node, value, valuelen) ;
  conftree_add(&node) ;
}

static inline void parse_global (char const *s, size_t const *word, size_t n, mdt const *md)
{
  static char const *const globalkeys[] =
  {
    "XXX_no_translate",
    "cgi_timeout",
    "executable_means_cgi",
    "max_cgi_body_length",
    "max_request_body_length",
    "read_timeout",
    "write_timeout"
  } ;
  char const *const *x ;
  uint32_t u ;
  char pack[4] ;
  if (n != 2)
    strerr_dief8x(1, "too ", n < 2 ? "few" : "many", " arguments to directive ", "global", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  x = BSEARCH(char const *, s + word[0], globalkeys) ;
  if (!x) strerr_dief6x(1, "unrecognized global setting ", s + word[0], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!uint320_scan(s + word[1], &u))
    strerr_dief6x(1, "invalid (non-numeric) value for global setting ", s + word[0], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  uint32_pack_big(pack, u) ;
  {
    size_t len = strlen(*x) ;
    char key[len + 3] ;
    memcpy(key, "G:", 2) ;
    memcpy(key + 2, *x, len + 1) ;
    add_unique(key, pack, 4, md) ;
  }
}

static inline void parse_indexfile (char const *s, size_t const *word, size_t n, mdt const *md)
{
  node node ;
  if (!n)
    strerr_dief8x(1, "too ", "few", " arguments to directive ", "index_file", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  conftree_checkunique("G:index-file", md) ;
  confnode_start(&node, "G:index-file", md->filepos, md->line) ;
  for (size_t i = 0 ; i < n ; i++)
    confnode_add(&node, s + word[i], strlen(s + word[i]) + 1) ;
  conftree_add(&node) ;
}

static inline void parse_log (char const *s, size_t const *word, size_t n, mdt const *md)
{
  static struct namevalue_s const logkeys[] =
  {
    { .name = "answer", .value = TIPIDEE_LOG_ANSWER },
    { .name = "answer_size", .value = TIPIDEE_LOG_SIZE },
    { .name = "debug", .value = TIPIDEE_LOG_DEBUG },
    { .name = "host_as_prefix", .value = TIPIDEE_LOG_HOSTASPREFIX },
    { .name = "hostname", .value = TIPIDEE_LOG_CLIENTHOST },
    { .name = "ip", .value = TIPIDEE_LOG_CLIENTIP },
    { .name = "nothing", .value = 0 },
    { .name = "referrer", .value = TIPIDEE_LOG_REFERRER },
    { .name = "request", .value = TIPIDEE_LOG_REQUEST },
    { .name = "resource", .value = TIPIDEE_LOG_RESOURCE },
    { .name = "start", .value = TIPIDEE_LOG_START },
    { .name = "user-agent", .value = TIPIDEE_LOG_UA },
    { .name = "x-forwarded-for", .value = TIPIDEE_LOG_XFORWARDEDFOR }
  } ;
  uint32_t v = 0 ;
  char pack[4] ;
  if (!n)
    strerr_dief8x(1, "too ", "few", " arguments to directive ", "log", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  for (size_t i = 0 ; i < n ; i++)
  {
    struct namevalue_s const *l = BSEARCH(struct namevalue_s, s + word[i], logkeys) ;
    if (!l) strerr_dief6x(1, "unrecognized log setting ", s + word[i], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
    if (l->value) v |= l->value ;
    else if (n == 1) v = 0 ;
    else strerr_dief5x(1, "log nothing cannot be mixed with other log values", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  }
  
  uint32_pack_big(pack, v) ;
  add_unique("G:logv", pack, 4, md) ;
}

static inline void parse_contenttype (char const *s, size_t const *word, size_t n, mdt const *md)
{
  char const *ct ;
  if (n < 2)
    strerr_dief8x(1, "too ", "few", " arguments to directive ", "content-type", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  ct = s + *word++ ;
  if (!strchr(ct, '/'))
    strerr_dief6x(1, "Content-Type must include a slash", " - check directive content-type", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  n-- ;
  for (size_t i = 0 ; i < n ; i++)
  {
    if (s[word[i]] == '\"' && s[word[i]+1] == '\"' && !s[word[i]+2])
      add_unique("T:", ct, strlen(ct) + 1, md) ;
    else if (s[word[i]] != '.')
      strerr_dief6x(1, "file extensions must be \"\" or start with a dot", " - check directive content-type", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
    else
    {
      size_t len = strlen(s + word[i]) ;
      char key[len + 2] ;
      key[0] = 'T' ;
      key[1] = ':' ;
      memcpy(key + 2, s + word[i] + 1, len - 1) ;
      key[len + 1] = 0 ;
      add_unique(key, ct, strlen(ct) + 1, md) ;
    }
  }
}

static inline void parse_customheader (char const *s, size_t const *word, size_t n, mdt const *md)
{
  static struct namevalue_s const choptions[] =
  {
    { .name = "add", .value = CHO_ADD },
    { .name = "always", .value = CHO_ALWAYS },
    { .name = "never", .value = CHO_NEVER },
    { .name = "remove", .value = CHO_REMOVE },
  } ;
  struct namevalue_s const *p ;
  if (n < 2)
    strerr_dief8x(1, "too ", "few", " arguments to directive ", "custom-header", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  p = BSEARCH(struct namevalue_s, s + *word, choptions) ;
  if (!p) strerr_dief9x(1, "invalid subcommand ", s + *word, " for", " directive ", "custom-header", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if ((p->value == CHO_ADD || p->value == CHO_ALWAYS) == (n == 2))
    strerr_dief10x(1, "too ", n == 2 ? "few" : "many", " arguments to directive ", "custom-header", " ", p->name, " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  word++ ; n-- ;
  {
    size_t len = strlen(s + *word) ;
    char key[len + 1] ;
    memcpy(key, s + *word++, len + 1) ; n-- ;
    header_canonicalize(key) ;
    if (!header_allowed(key))
      strerr_dief7x(1, "header ", key, " cannot be customized", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
    headers_checkunique(key, md) ;
    switch (p->value)
    {
      case CHO_ADD :
        headers_addv(key, 1, s, word, n, md->filepos, md->line) ;
        break ;
      case CHO_ALWAYS :
        headers_addv(key, 0, s, word, n, md->filepos, md->line) ;
        break ;
      case CHO_NEVER :
        headers_addv(key, 0, 0, 0, 0, md->filepos, md->line) ;
        break ;
      case CHO_REMOVE :
        headers_addv(key, 1, 0, 0, 0, md->filepos, md->line) ;
        break ;
      default : strerr_dief1x(101, "can't happen: unknown value in parse_customheader") ;
    }
  }
}

static inline void parse_noredirect (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md)
{
  if (n != 1)
    strerr_dief8x(1, "too ", n > 1 ? "many" : "few", " arguments to directive ", "noredirect", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief6x(1, "noredirection", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (s[*word] != '/')
    strerr_dief6x(1, "noredirected resource", " must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    size_t urlen = strlen(s + *word) ;
    char key[3 + domainlen + urlen] ;
    if (s[*word + urlen - 1] == '/') { key[0] = 'r' ; urlen-- ; } else key[0] = 'R' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + *word, urlen) ;
    key[2 + domainlen + urlen] = 0 ;
    add_unique(key, " ", 2, md) ;
  }
}

static inline void parse_redirect (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md)
{
  static uint32_t const codes[4] = { 307, 308, 302, 301 } ;
  uint32_t code ;
  uint32_t i = 0 ;
  if (n != 3)
    strerr_dief8x(1, "too ", n > 3 ? "many" : "few", " arguments to directive ", "redirect", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief6x(1, "redirection", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (s[word[0]] != '/')
    strerr_dief6x(1, "redirected resource", " must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!uint320_scan(s + word[1], &code))
    strerr_dief6x(1, "invalid redirection code ", s + word[1], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  for (; i < 4 ; i++) if (code == codes[i]) break ;
  if (i >= 4)
    strerr_dief6x(1, "redirection code ", "must be 301, 302, 307 or 308", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (strncmp(s + word[2], "http://", 7) && strncmp(s + word[2], "https://", 8))
    strerr_dief5x(1, "redirection target must be a full http:// or https:// target", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    node node ;
    size_t urlen = strlen(s + word[0]) ;
    char key[3 + domainlen + urlen] ;
    if (s[word[0] + urlen - 1] == '/') { key[0] = 'r' ; urlen-- ; } else key[0] = 'R' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + word[0], urlen) ;
    key[2 + domainlen + urlen] = 0 ;
    conftree_checkunique(key, md) ;
    confnode_start(&node, key, md->filepos, md->line) ;
    key[0] = '@' | i ;
    confnode_add(&node, &key[0], 1) ;
    urlen = strlen(s + word[2]) ;
    confnode_add(&node, s + word[2], urlen - (s[word[2] + urlen - 1] == '/')) ;
    confnode_add(&node, "", 1) ;
    conftree_add(&node) ;
  }
}

static void parse_bitattr (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md, uint8_t bit, int h)
{
  static char const *const attr[3][2] = { { "noncgi", "cgi" }, { "nonnph", "nph", }, { "noauth", "basic-auth" } } ;
  if (n != 1)
    strerr_dief8x(1, "too ", n > 1 ? "many" : "few", " arguments to directive ", attr[bit][h], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief7x(1, "resource attribute ", "definition", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (s[*word] != '/')
    strerr_dief6x(1, "resource", " must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    node const *oldnode ;
    size_t arglen = strlen(s + *word) ;
    char key[3 + domainlen + arglen] ;
    if (s[*word + arglen - 1] == '/') { key[0] = 'a' ; arglen-- ; } else key[0] = 'A' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + *word, arglen) ;
    key[2 + domainlen + arglen] = 0 ;
    oldnode = conftree_search(key) ;
    if (oldnode)
    {
      uint32_t flags, mask ;
      uint32_unpack_big(g.storage.s + oldnode->data, &flags) ;
      uint32_unpack_big(g.storage.s + oldnode->data + 4, &mask) ;
      if (mask & 1 << bit)
      {
        char fmt[UINT32_FMT] ;
        fmt[uint32_fmt(fmt, oldnode->line)] = 0 ;
        strerr_diefn(1, 13, "resource attribute ", attr[bit][h], " redefinition", " in file ", g.storage.s + md->filepos, " line ", md->linefmt, "; previous definition", " in file ", g.storage.s + oldnode->filepos, " line ", fmt, " or later") ;
      }
      mask |= 1 << bit ;
      if (h) flags |= 1 << bit ; else flags &= ~(1 << bit) ;
      uint32_pack_big(g.storage.s + oldnode->data, flags) ;
      uint32_pack_big(g.storage.s + oldnode->data + 4, mask) ;
    }
    else
    {
      node node ;
      uint32_t flags = h ? 1 << bit : 0 ;
      uint32_t mask = 1 << bit ;
      char val[9] = "\0\0\0\0\0\0\0\0" ;
      uint32_pack_big(val, flags) ;
      uint32_pack_big(val + 4, mask) ;
      confnode_start(&node, key, md->filepos, md->line) ;
      confnode_add(&node, val, 9) ;
      conftree_add(&node) ;
    }
  }
}

static inline void parse_filetype (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md)
{
  if (n != 2)
    strerr_dief8x(1, "too ", n > 2 ? "many" : "few", " arguments to directive ", "file-type", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief7x(1, "file-type", " definition", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (s[word[0]] != '/')
    strerr_dief6x(1, "resource", " must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    node const *oldnode ;
    size_t arglen = strlen(s + word[0]) ;
    char key[3 + domainlen + arglen] ;
    if (s[word[0] + arglen - 1] == '/') { key[0] = 'a' ; arglen-- ; } else key[0] = 'A' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + *word, arglen) ;
    key[2 + domainlen + arglen] = 0 ;
    oldnode = conftree_search(key) ;
    if (oldnode)
    {
      if (g.storage.s[oldnode->data + 8])
      {
        char fmt[UINT32_FMT] ;
        fmt[uint32_fmt(fmt, oldnode->line)] = 0 ;
        strerr_diefn(1, 12, "file-type", " redefinition", " in file ", g.storage.s + md->filepos, " line ", md->linefmt, "; previous definition", " in file ", g.storage.s + oldnode->filepos, " line ", fmt, " or later") ;
      }
      else
      {
        node node ;
        confnode_start(&node, key, md->filepos, md->line) ;
        confnode_add(&node, g.storage.s + oldnode->data, 8) ;
        confnode_add(&node, s + word[1], strlen(s + word[1]) + 1) ;
        conftree_update(&node) ;
      }
    }
    else
    {
      node node ;
      confnode_start(&node, key, md->filepos, md->line) ;
      confnode_add(&node, "\0\0\0\0\0\0\0", 8) ;
      confnode_add(&node, s + word[1], strlen(s + word[1]) + 1) ;
      conftree_add(&node) ;
    }
  }
}

static inline void parse_customresponse (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md)
{
  uint32_t status ;
  size_t word1, len ;
  char key[7 + domainlen] ;

  if (n != 2)
    strerr_dief8x(1, "too ", n > 2 ? "many" : "few", " arguments to directive ", "custom-response", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief7x(1, "custom-response", " definition", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!uint320_scan(s + word[0], &status) || status < 100 || status > 999)
    strerr_dief6x(1, "invalid status", " for custom-response", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  word1 = word[1] ;
  while (s[word1] == '/') word1++ ;
  len = strlen(s + word1) ;
  if (!len || !strncmp(s + word1, "../", 3) || strstr(s + word1, "/../") || (len >= 3 && !strcmp(s + word1 + len - 3, "/..")))
    strerr_dief6x(1, "invalid file", " for custom-response", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  key[0] = 'E' ;
  key[1] = ':' ;
  uint32_fmt(key + 2, status) ;
  key[5] = ':' ;
  memcpy(key + 6, domain, domainlen) ;
  key[6 + domainlen] = 0 ;
  add_unique(key, s + word1, len + 1, md) ;
}

static inline void process_line (char const *s, size_t const *word, size_t n, stralloc *domain, mdt *md)
{
  static struct namevalue_s const directives[] =
  {
    { .name = "!", .value = T_BANG },
    { .name = "autochunk", .value = T_AUTOCHUNK },
    { .name = "basic-auth", .value = T_BASICAUTH },
    { .name = "cgi", .value = T_CGI },
    { .name = "content-type", .value = T_CONTENTTYPE },
    { .name = "custom-header", .value = T_CUSTOMHEADER },
    { .name = "custom-response", .value = T_CUSTOMRESPONSE },
    { .name = "domain", .value = T_DOMAIN },
    { .name = "file-type", .value = T_FILETYPE },
    { .name = "global", .value = T_GLOBAL },
    { .name = "index-file", .value = T_INDEXFILE },
    { .name = "log", .value = T_LOG },
    { .name = "no-auth", .value = T_NOAUTH },
    { .name = "noautochunk", .value = T_NOAUTOCHUNK },
    { .name = "noncgi", .value = T_NONCGI },
    { .name = "nonnph", .value = T_NONNPH },
    { .name = "noredirect", .value = T_NOREDIRECT },
    { .name = "nph", .value = T_NPH },
    { .name = "nph-prefix", .value = T_NPHPREFIX },
    { .name = "redirect", .value = T_REDIRECT },
  } ;
  struct namevalue_s const *directive ;
  char const *word0 ;
  if (!n--) return ;
  word0 = s + *word++ ;
  directive = BSEARCH(struct namevalue_s, word0, directives) ;
  if (!directive)
    strerr_dief6x(1, "unrecognized word ", word0, " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  switch (directive->value)
  {
    case T_BANG :
    {
      size_t len, r, w ;
      if (n != 2 || !uint320_scan(s + word[0], &md->line))
        strerr_dief5x(101, "can't happen: invalid ! control directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      len = strlen(s + word[1]) ;
      if (!stralloc_readyplus(&g.storage, len + 1)) dienomem() ;
      if (!string_unquote(g.storage.s + g.storage.len, &w, s + word[1], len, &r))
        strerr_dief7sys(101, "can't happen: unable to unquote ", s + word[1], " in file ", g.storage.s + md->filepos, " line ", md->linefmt, ", error reported is") ;
      g.storage.s[g.storage.len + w++] = 0 ;
      md->filepos = g.storage.len ;
      g.storage.len += w ;
      break ;
    }
    case T_GLOBAL :
      parse_global(s, word, n, md) ;
      break ;
    case T_INDEXFILE :
      parse_indexfile(s, word, n, md) ;
      break ;
    case T_LOG :
      parse_log(s, word, n, md) ;
      break ;
    case T_CONTENTTYPE :
      parse_contenttype(s, word, n, md) ;
      break ;
    case T_CUSTOMHEADER :
      parse_customheader(s, word, n, md) ;
      break ;
    case T_DOMAIN :
      if (n != 1)
        strerr_dief8x(1, "too ", n > 1 ? "many" : "few", " arguments to directive ", "domain", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      domain->len = 0 ;
      if (!stralloc_cats(domain, s + *word)) dienomem() ;
      while (domain->len && (domain->s[domain->len - 1] == '/' || domain->s[domain->len - 1] == '.')) domain->len-- ;
      break ;
    case T_NPHPREFIX :
      if (n != 1)
        strerr_dief8x(1, "too ", n > 1 ? "many" : "few", " arguments to directive ", "nph-prefix", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      if (!domain->s)
        strerr_dief6x(1, "nph prefix definition", "without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      if (strchr(s + *word, '/')) strerr_dief5x(1, "invalid / in nph-prefix argument", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      {
        char key[3 + domain->len] ;
        key[0] = 'N' ;
        key[1] = ':' ;
        memcpy(key + 2, domain->s, domain->len) ;
        key[2 + domain->len] = 0 ;
        add_unique(key, s + *word, strlen(s + *word) + 1, md) ;
      }
      break ;
    case T_REDIRECT :
      parse_redirect(s, word, n, domain->s, domain->len, md) ;
      break ;
    case T_NOREDIRECT :
      parse_noredirect(s, word, n, domain->s, domain->len, md) ;
      break ;
    case T_CGI :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 0, 1) ;
      break ;
    case T_NONCGI :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 0, 0) ;
      break ;
    case T_NPH :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 1, 1) ;
      break ;
    case T_NONNPH :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 1, 0) ;
      break ;
    case T_BASICAUTH :
      strerr_warnw5x("file ", g.storage.s + md->filepos, " line ", md->linefmt, ": directive basic-auth not implemented in tipidee-" TIPIDEE_VERSION) ;
      parse_bitattr(s, word, n, domain->s, domain->len, md, 2, 1) ;
      break ;
    case T_NOAUTH :
      strerr_warnw5x("file ", g.storage.s + md->filepos, " line ", md->linefmt, ": directive basic-auth not implemented in tipidee-" TIPIDEE_VERSION) ;
      parse_bitattr(s, word, n, domain->s, domain->len, md, 2, 0) ;
      break ;
    case T_AUTOCHUNK :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 3, 1) ;
      break ;
    case T_NOAUTOCHUNK :
      parse_bitattr(s, word, n, domain->s, domain->len, md, 3, 0) ;
      break ;
    case T_FILETYPE :
      parse_filetype(s, word, n, domain->s, domain->len, md) ;
      break ;
    case T_CUSTOMRESPONSE :
      parse_customresponse(s, word, n, domain->s, domain->len, md) ;
      break ;
  }
}

static inline uint8_t cclass (char c)
{
  switch (c)
  {
    case 0 : return 0 ;
    case ' ' :
    case '\t' :
    case '\f' :
    case '\r' : return 1 ;
    case '#' : return 2 ;
    case '\n' : return 3 ;
    default : return 4 ;
  }
}

static inline char next (buffer *b, mdt const *md)
{
  char c ;
  ssize_t r = buffer_get(b, &c, 1) ;
  if (r == -1) strerr_diefu1sys(111, "read from preprocessor") ;
  if (!r) return 0 ;
  if (!c) strerr_dief5x(1, "null character", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  return c ;
}

void conf_lexparse (buffer *b, char const *ifile)
{
  static uint8_t const table[4][5] =  /* see PARSING-config.txt */
  {
    { 0x04, 0x02, 0x01, 0x80, 0x33 },
    { 0x04, 0x01, 0x01, 0x80, 0x01 },
    { 0x84, 0x02, 0x01, 0x80, 0x33 },
    { 0xc4, 0x42, 0x23, 0xc0, 0x23 }
  } ;
  stralloc sa = STRALLOC_ZERO ;
  genalloc words = GENALLOC_ZERO ; /* size_t */
  stralloc domain = STRALLOC_ZERO ;
  mdt md = MDT_ZERO ;
  uint8_t state = 0 ;
  if (!stralloc_catb(&g.storage, ifile, strlen(ifile) + 1)) dienomem() ;
  while (state < 0x04)
  {
    char c = next(b, &md) ;
    uint8_t what = table[state][cclass(c)] ;
    state = what & 0x07 ;
    if (what & 0x10) if (!genalloc_catb(size_t, &words, &sa.len, 1)) dienomem() ;
    if (what & 0x20) if (!stralloc_catb(&sa, &c, 1)) dienomem() ; 
    if (what & 0x40) if (!stralloc_0(&sa)) dienomem() ;
    if (what & 0x80)
    {
      process_line(sa.s, genalloc_s(size_t, &words), genalloc_len(size_t, &words), &domain, &md) ;
      genalloc_setlen(size_t, &words, 0) ;
      sa.len = 0 ;
      md.line++ ;
      md.linefmt[uint32_fmt(md.linefmt, md.line)] = 0 ;
    }
  }
  stralloc_free(&domain) ;
  genalloc_free(size_t, &words) ;
  stralloc_free(&sa) ;
  headers_finish() ;
}
