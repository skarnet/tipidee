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

#include <tipidee/config.h>
#include "tipidee-config-internal.h"

#define dienomem() strerr_diefu1sys(111, "stralloc_catb")
#define dietoobig() strerr_diefu1sys(100, "read configuration")

typedef struct mdt_s mdt, *mdt_ref ;
struct mdt_s
{
  size_t filepos ;
  uint32_t line ;
  char linefmt[UINT32_FMT] ;
} ;
#define MDT_ZERO { .filepos = 0, .line = 0, .linefmt = "0" }

struct globalkey_s
{
  char const *name ;
  char const *key ;
  uint32_t type ;
} ;

static int globalkey_cmp (void const *a, void const *b)
{
  return strcmp((char const *)a, ((struct globalkey_s const *)b)->name) ;
}

enum token_e
{
  T_BANG,
  T_GLOBAL,
  T_CONTENTTYPE,
  T_DOMAIN,
  T_NPHPREFIX,
  T_REDIRECT,
  T_CGI,
  T_NONCGI,
  T_NPH,
  T_NONNPH,
  T_BASICAUTH,
  T_NOAUTH,
  T_FILETYPE
} ;

struct directive_s
{
  char const *s ;
  enum token_e token ;
} ;

static int directive_cmp (void const *a, void const *b)
{
  return strcmp((char const *)a, ((struct directive_s const *)b)->s) ;
}

static void check_unique (char const *key, mdt const *md)
{
  confnode const *node = conftree_search(key) ;
  if (node)
  {
    char fmt[UINT32_FMT] ;
    fmt[uint32_fmt(fmt, node->line)] = 0 ;
    strerr_diefn(1, 11, "duplicate key ", key, " in file ", g.storage.s + md->filepos, " line ", md->linefmt, ", previously defined", " in file ", g.storage.s + node->filepos, " line ", fmt) ;
  }
}

static void add_unique (char const *key, char const *value, size_t valuelen, mdt const *md)
{
  confnode node ;
  check_unique(key, md) ;
  confnode_start(&node, key, md->filepos, md->line) ;
  confnode_add(&node, value, valuelen) ;
  conftree_add(&node) ;
}

static inline void parse_global (char const *s, size_t const *word, size_t n, mdt const *md)
{
  static struct globalkey_s const globalkeys[] =
  {
    { .name = "cgi_timeout", .key = "G:cgi_timeout", .type = 0 },
    { .name = "index_file", .key = "G:index_file", .type = 1 },
    { .name = "max_cgi_body_length", .key = "G:max_cgi_body_length", .type = 0 },
    { .name = "max_request_body_length", .key = "G:max_request_body_length", .type = 0 },
    { .name = "read_timeout", .key = "G:read_timeout", .type = 0 },
    { .name = "verbosity", .key = "G:verbosity", .type = 0 },
    { .name = "write_timeout", .key = "G:write_timeout", .type = 0 }
  } ;
  struct globalkey_s *gl ;
  if (n < 2)
    strerr_dief8x(1, "too ", "few", " arguments to directive ", "global", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  gl = bsearch(s + word[0], globalkeys, sizeof(globalkeys)/sizeof(struct globalkey_s), sizeof(struct globalkey_s), &globalkey_cmp) ;
  if (!gl) strerr_dief6x(1, "unrecognized global setting ", s + word[0], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  switch (gl->type)
  {
    case 0 : /* 4 bytes */
    {
      char pack[4] ;
      uint32_t u ;
      if (n > 2)
        strerr_dief8x(1, "too ", "many", " arguments to global setting ", s + word[0], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      if (!uint320_scan(s + word[1], &u))
        strerr_dief6x(1, "invalid (non-numeric) value for global setting ", s + word[0], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
      uint32_pack_big(pack, u) ;
      add_unique(gl->key, pack, 4, md) ;
      break ;
    }
    case 1 : /* argv */
    {
      confnode node ;
      check_unique(gl->key, md) ;
      confnode_start(&node, gl->key, md->filepos, md->line) ;
      for (size_t i = 1 ; i < n ; i++)
        confnode_add(&node, s + word[i], strlen(s + word[i]) + 1) ;
      conftree_add(&node) ;
      break ;
    }
  }
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
    size_t len = strlen(s + word[i]) ;
    char key[len + 2] ;
    if (s[word[i]] != '.')
      strerr_dief6x(1, "file extensions must start with a dot", " - check directive content-type", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
    key[0] = 'T' ;
    key[1] = ':' ;
    memcpy(key + 2, s + word[i] + 1, len - 1) ;
    key[len + 1] = 0 ;
    add_unique(key, ct, strlen(ct) + 1, md) ;
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
    strerr_dief5x(1, "redirected resource must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!uint320_scan(s + word[1], &code))
    strerr_dief6x(1, "invalid redirection code ", s + word[1], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  for (; i < 4 ; i++) if (code == codes[i]) break ;
  if (i >= 4)
    strerr_dief6x(1, "redirection code ", "must be 301, 302, 307 or 308", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (strncmp(s + word[2], "http://", 7) && strncmp(s + word[2], "https://", 8))
    strerr_dief5x(1, "redirection target must be a full http:// or https:// target", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    confnode node ;
    size_t urlen = strlen(s + word[0]) ;
    char key[3 + domainlen + urlen] ;
    if (s[word[0] + urlen - 1] == '/') { key[0] = 'r' ; urlen-- ; } else key[0] = 'R' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + word[0], urlen) ;
    key[2 + domainlen + urlen] = 0 ;
    check_unique(key, md) ;
    confnode_start(&node, key, md->filepos, md->line) ;
    key[0] = '@' | i ;
    confnode_add(&node, &key[0], 1) ;
    confnode_add(&node, s + word[2], strlen(s + word[2]) + 1) ;
    conftree_add(&node) ;
  }
}

static void parse_bitattr (char const *s, size_t const *word, size_t n, char const *domain, size_t domainlen, mdt const *md, unsigned int bit, int set)
{
  static char const *attr[3][2] = { { "noncgi", "cgi" }, { "nonnph", "nph", }, { "noauth", "basic-auth" } } ;
  uint8_t mask = (uint8_t)0x01 << bit ;
  if (n != 1)
    strerr_dief8x(1, "too ", n > 1 ? "many" : "few", " arguments to directive ", attr[bit][set], " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (!domain)
    strerr_dief7x(1, "resource attribute ", "definition", " without a domain directive", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  if (s[*word] != '/')
    strerr_dief5x(1, "resource must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    confnode const *oldnode ;
    size_t arglen = strlen(s + *word) ;
    char key[3 + domainlen + arglen] ;
    if (s[*word + arglen - 1] == '/') { key[0] = 'a' ; arglen-- ; } else key[0] = 'A' ;
    key[1] = ':' ;
    memcpy(key + 2, domain, domainlen) ;
    memcpy(key + 2 + domainlen, s + *word, arglen) ;
    key[2 + domainlen + arglen] = 0 ;
    oldnode = conftree_search(key) ;
    if (oldnode)
      if (g.storage.s[oldnode->data] & mask)
      {
        char fmt[UINT32_FMT] ;
        fmt[uint32_fmt(fmt, oldnode->line)] = 0 ;
        strerr_diefn(1, 13, "resource attribute ", attr[bit][set], " redefinition", " in file ", g.storage.s + md->filepos, " line ", md->linefmt, "; previous definition", " in file ", g.storage.s + oldnode->filepos, " line ", fmt, " or later") ;
      }
      else
      {
        g.storage.s[oldnode->data] |= mask ;
        if (set) g.storage.s[oldnode->data + 1] |= mask ;
        else g.storage.s[oldnode->data + 1] &= ~mask ;
      }
    else
    {
      confnode node ;
      char val[3] = { mask, set ? mask : 0, 0 } ;
      confnode_start(&node, key, md->filepos, md->line) ;
      confnode_add(&node, val, 3) ;
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
    strerr_dief5x(1, "resource must start with /", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  {
    confnode const *oldnode ;
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
      if (g.storage.s[oldnode->data] & 0x80)
      {
        char fmt[UINT32_FMT] ;
        fmt[uint32_fmt(fmt, oldnode->line)] = 0 ;
        strerr_diefn(1, 12, "file-type", " redefinition", " in file ", g.storage.s + md->filepos, " line ", md->linefmt, "; previous definition", " in file ", g.storage.s + oldnode->filepos, " line ", fmt, " or later") ;
      }
      
      else
      {
        confnode node ;
        char val[2] = { g.storage.s[oldnode->data] | 0x80, g.storage.s[oldnode->data + 1] } ;
        confnode_start(&node, key, md->filepos, md->line) ;
        confnode_add(&node, val, 2) ;
        confnode_add(&node, s + word[1], strlen(s + word[1]) + 1) ;
        conftree_update(&node) ;
      }
    }
    else
    {
      confnode node ;
      char val[2] = { 0x80, 0x00 } ;
      confnode_start(&node, key, md->filepos, md->line) ;
      confnode_add(&node, val, 2) ;
      confnode_add(&node, s + word[1], strlen(s + word[1]) + 1) ;
      conftree_add(&node) ;
    }
  }
}

static inline void process_line (char const *s, size_t const *word, size_t n, stralloc *domain, mdt *md)
{
  static struct directive_s const directives[] =
  {
    { .s = "!", .token = T_BANG },
    { .s = "basic-auth", .token = T_BASICAUTH },
    { .s = "cgi", .token = T_CGI },
    { .s = "content-type", .token = T_CONTENTTYPE },
    { .s = "domain", .token = T_DOMAIN },
    { .s = "file-type", .token = T_FILETYPE },
    { .s = "global", .token = T_GLOBAL },
    { .s = "no-auth", .token = T_NOAUTH },
    { .s = "noncgi", .token = T_NONCGI },
    { .s = "nonnph", .token = T_NONNPH },
    { .s = "nph", .token = T_NPH },
    { .s = "nph-prefix", .token = T_NPHPREFIX },
    { .s = "redirect", .token = T_REDIRECT },
  } ;
  struct directive_s const *directive ;
  char const *word0 ;
  if (!n--) return ;
  word0 = s + *word++ ;
  directive = bsearch(word0, directives, sizeof(directives)/sizeof(struct directive_s), sizeof(struct directive_s), &directive_cmp) ;
  if (!directive)
    strerr_dief6x(1, "unrecognized word ", word0, " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
  switch (directive->token)
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
    case T_CONTENTTYPE :
      parse_contenttype(s, word, n, md) ;
      break ;
    case T_DOMAIN :
      if (n != 1)
        strerr_dief8x(1, "too", n > 1 ? "many" : "few", " arguments to directive ", "domain", " in file ", g.storage.s + md->filepos, " line ", md->linefmt) ;
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
    case T_FILETYPE :
      parse_filetype(s, word, n, domain->s, domain->len, md) ;
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
  static uint8_t const table[4][5] =  /* see PARSING.txt */
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
}
