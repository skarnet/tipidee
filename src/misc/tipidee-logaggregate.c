/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <skalibs/uint64.h>
#include <skalibs/bytestr.h>
#include <skalibs/types.h>
#include <skalibs/buffer.h>
#include <skalibs/prog.h>
#include <skalibs/gol.h>
#include <skalibs/strerr.h>
#include <skalibs/tai.h>
#include <skalibs/fmtscan.h>
#include <skalibs/stralloc.h>
#include <skalibs/skamisc.h>
#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>
#include <skalibs/lolstdio.h>

#define USAGE "tipidee-logaggregate [ -4 | -6 ]"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem() strerr_diefu1sys(111, "stralloc_catb")
#define dieout() strerr_diefu1sys(111, "write to stdout")
#define dietoobig() strerr_dief1x(100, "too much data to process")

enum golb_e
{
  GOLB_IS6 = 0x01
} ;

static uint64_t wgolb = 0 ;
static uint32_t line = 1 ;


 /* pidip */

typedef struct pidip_s pidip, pidip_ref ;
struct pidip_s
{
  pid_t pid ;
  char ip[16] ;
} ;
#define PIDIP_ZERO { .pid = 0, .ip = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" }

static void *pidip_dtok (uint32_t d, void *data)
{
  gensetdyn *g = data ;
  pidip *p = GENSETDYN_P(pidip, g, d) ;
  return &p->pid ;
}

static int pid_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  pid_t aa = *(pid_t const *)a ;
  pid_t bb = *(pid_t const *)b ;
  return aa < bb ? -1 : aa > bb ;
}

static gensetdyn pidip_list = GENSETDYN_INIT(pidip, 16, 3, 8) ;
static avltree pidip_map = AVLTREE_ZERO ;
#define PIDIP(d) GENSETDYN_P(pidip, &pidip_list, (d))

static int pidip_search (pid_t pid, uint32_t *d)
{
  if (!avltree_search(&pidip_map, &pid, d))
  {
    char fmtline[UINT32_FMT] ;
    char fmt[PID_FMT] ;
    fmtline[uint32_fmt(fmtline, line)] = 0 ;
    fmt[pid_fmt(fmt, pid)] = 0 ;
    strerr_warnw("line ", fmtline, ": pid ", fmt, " not registered") ;
    return 0 ;
  }
  return 1 ;
}

static void pidip_delete (pid_t pid)
{
  uint32_t d ;
  if (!pidip_search(pid, &d)) return ;
  avltree_delete(&pidip_map, &pid) ;
  gensetdyn_delete(&pidip_list, d) ;
}

static void pidip_add (pid_t pid, char const *ip)
{
  uint32_t d ;
  if (avltree_search(&pidip_map, &pid, &d))
  {
    char fmtline[UINT32_FMT] ;
    char fmt[PID_FMT] ;
    fmtline[uint32_fmt(fmtline, line)] = 0 ;
    fmt[pid_fmt(fmt, pid)] = 0 ;
    strerr_dief(1, "line ", fmtline, ": pid ", fmt, " started but was already registered") ;
  }
  if (!gensetdyn_new(&pidip_list, &d)) dienomem() ;
  pidip *p = GENSETDYN_P(pidip, &pidip_list, d) ;
  p->pid = pid ;
  memcpy(p->ip, ip, wgolb & GOLB_IS6 ? 16 : 4) ;
  if (!avltree_insert(&pidip_map, d)) dienomem() ;
}


 /* ipinfo */

typedef struct ipinfo_s ipinfo, *ipinfo_ref ;
struct ipinfo_s
{
  char ip[16] ;
  genalloc indices ;  /* uint32_t */
} ;
#define IPINFO_ZERO { .indices = GENALLOC_ZERO }

static void *ipinfo_dtok (uint32_t d, void *data)
{
  genalloc *g = data ;
  ipinfo *p = genalloc_s(ipinfo, g) + d ;
  return p->ip ;
}

static int ip_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  return memcmp(a, b, wgolb & GOLB_IS6 ? 16 : 4) ;
}


static genalloc ipinfo_list = GENALLOC_ZERO ;  /* ipinfo */
static avltree ipinfo_map = AVLTREE_ZERO ;
#define IPINFO(d) (genalloc_s(ipinfo, &ipinfo_list) + (d))


 /* storage */

static int string_cmp (void const *a, void const *b, void *data)
{
  (void)data ;
  return strcmp((char const *)a, (char const *)b) ;
}

static void *string_dtok (uint32_t d, void *data)
{
  stralloc *sa = data ;
  return sa->s + d ;
}

static stralloc storage = STRALLOC_ZERO ;
static avltree storage_map = AVLTREE_ZERO ;

static void add_request (pid_t pid, char const *host, char const *path, char const *query)
{
  ipinfo *info ;
  pidip *p ;
  uint32_t d ;
  size_t pos ;
  if (!pidip_search(pid, &d)) return ;
  p = PIDIP(d) ;
  if (!avltree_search(&ipinfo_map, p->ip, &d))
  {
    ipinfo i = IPINFO_ZERO ;
    memcpy(i.ip, p->ip, wgolb & GOLB_IS6 ? 16 : 4) ;
    d = genalloc_len(ipinfo, &ipinfo_list) ;
    if (!genalloc_append(ipinfo, &ipinfo_list, &i)) dienomem() ;
    if (!avltree_insert(&ipinfo_map, d)) dienomem() ;
  }
  info = IPINFO(d) ;
  pos = storage.len ;
  if (pos > UINT32_MAX) dietoobig() ;
  if (!stralloc_cats(&storage, host)
   || !stralloc_catb(&storage, ":", 1)
   || !stralloc_cats(&storage, path)) dienomem() ;
  if (query)
  {
    if (!stralloc_catb(&storage, "?", 1)
     || !stralloc_cats(&storage, query)) dienomem() ;
  }
  if (!stralloc_0(&storage)) dienomem() ;
  if (!avltree_search(&storage_map, storage.s + pos, &d))
  {
    d = pos ;
    if (!avltree_insert(&storage_map, d)) dienomem() ;
  }
  else storage.len = pos ;
  if (!genalloc_append(uint32_t, &info->indices, &d)) dienomem() ;
}

 /* parse */

static void parse_answer (pid_t pid, char const *host, char *s)
{
  (void)pid ;
  (void)host ;
  (void)s ;
}

static void parse_exit (pid_t pid, char const *host, char *s)
{
  (void)s ;
  (void)host ;
  pidip_delete(pid) ;
}

static char *parse_host (char *s, char const **h)
{
  char *p ;
  if (strncmp(s, "host ", 5)) return 0 ;
  s += 5 ;
  p = strchr(s, ' ') ;
  if (!p) return 0 ;
  *p = 0 ;
  *h = s ;
  return p+1 ;
}

static void parse_request (pid_t pid, char const *host, char *s)
{
  char *p ;
  char *path = 0 ;
  char *query = 0 ;
  if (strncmp(s, "GET ", 4)) return ;
  s += 4 ;
  if (!host)
  {
    s = parse_host(s, &host) ;
    if (!host)
    {
      char fmtline[UINT32_FMT] ;
      fmtline[uint32_fmt(fmtline, line)] = 0 ;
      strerr_warnw("line ", fmtline, ": request line with invalid/no ", "host") ;
      return ;
    }
  }
  if (strncmp(s, "path \"", 6))
  {
    char fmtline[UINT32_FMT] ;
    fmtline[uint32_fmt(fmtline, line)] = 0 ;
    strerr_warnw("line ", fmtline, ": request line with invalid/no ", "path") ;
    return ;
  }
  s += 6 ;
  p = strchr(s, '\"') ;
  if (!p)
  {
    char fmtline[UINT32_FMT] ;
    fmtline[uint32_fmt(fmtline, line)] = 0 ;
    strerr_warnw("line ", fmtline, ": request line with invalid/no ", "path") ;
    return ;
  }
  path = s ;
  *p = 0 ;
  s = p+1 ;
  if (!strncmp(s, " query ", 7))
  {
    s += 7 ;
    query = s ;
    p = strchr(s, ' ') ;
    if (!p)
    {
      char fmtline[UINT32_FMT] ;
      fmtline[uint32_fmt(fmtline, line)] = 0 ;
      strerr_warnw("line ", fmtline, ": request line with invalid query") ;
      return ;
    }
    *p = 0 ;
    s = p+1 ;
  }
  add_request(pid, host, path, query) ;
}

static void parse_resource (pid_t pid, char const *host, char *s)
{
  (void)pid ;
  (void)host ;
  (void)s ;
}

static void parse_start (pid_t pid, char const *host, char *s)
{
  char ip[16] ;
  (void)host ;
  if (strncmp(s, "ip ", 3))
  {
    char fmtline[UINT32_FMT] ;
    fmtline[uint32_fmt(fmtline, line)] = 0 ;
    strerr_warnw("line ", fmtline, ": start directive without ip") ;
    return ;
  }
  s += 3 ;
  if (wgolb & GOLB_IS6)
  {
    if (!ip6_scan(s, ip))
    {
      char fmtline[UINT32_FMT] ;
      fmtline[uint32_fmt(fmtline, line)] = 0 ;
      strerr_warnw("line ", fmtline, ": invalid ipv6: ", s) ;
      return ;
    }
  }
  else
  {
    if (!ip4_scan(s, ip))
    {
      char fmtline[UINT32_FMT] ;
      fmtline[uint32_fmt(fmtline, line)] = 0 ;
      strerr_warnw("line ", fmtline, ": invalid ipv4") ;
      return ;
    }
  }
  pidip_add(pid, ip) ;
}

struct wordf_s
{
  char const *s ;
  void (*f)(pid_t, char const *, char *) ;
} ;

static int wordf_cmp (void const *a, void const *b)
{
  char const *s = ((struct wordf_s const *)b)->s ;
  return strncmp((char const *)a, s, strlen(s)) ;
}

#define WORDF_BSEARCH(key, array) bsearch(key, (array), sizeof(array)/sizeof(struct wordf_s), sizeof(struct wordf_s), &wordf_cmp)

static void parse (pid_t pid, char const *host, char *s)
{
  static struct wordf_s const words[5] =
  {
    { .s = "answer ", .f = &parse_answer },
    { .s = "exit ", .f = &parse_exit },
    { .s = "request ", .f = &parse_request },
    { .s = "resource ", .f = &parse_resource },
    { .s = "start ", .f = &parse_start }
  } ;
  size_t wlen ;
  struct wordf_s const *p = WORDF_BSEARCH(s, words) ;
  if (!p) return ;
  wlen = strlen(p->s) ;
  (*p->f)(pid, host, s + wlen) ;
}

 /* output */

static int print_iter (uint32_t d, unsigned int h, void *data)
{
  ipinfo *info = IPINFO(d) ;
  size_t n = genalloc_len(uint32_t, &info->indices) ;
  char fmt[IP6_FMT] ;
  char fsz[SIZE_FMT] ;
  fmt[wgolb & GOLB_IS6 ? ip6_fmt(fmt, info->ip) : ip4_fmt(fmt, info->ip) ] = 0 ;
  fsz[size_fmt(fsz, n)] = 0 ;
  if (buffer_puts(buffer_1, "ip ") < 0
   || buffer_puts(buffer_1, fmt) < 0
   || buffer_puts(buffer_1, " hits ") < 0
   || buffer_puts(buffer_1, fsz) < 0
   || buffer_put(buffer_1, "\n", 1) < 0) dieout() ;
  for (size_t i = 0 ; i < n ; i++)
  {
    if (buffer_puts(buffer_1, storage.s + genalloc_s(uint32_t, &info->indices)[i]) < 0
     || buffer_put(buffer_1, "\n", 1) < 0) dieout() ;
  }
  if (!buffer_putflush(buffer_1, "\n", 1)) dieout() ;
  (void)data ;
  return 1 ;
}


 /* main */

static gol_bool const rgolb[2] =
{
  { .so = '4', .lo = "ipv4", .clear = GOLB_IS6, .set = 0 },
  { .so = '6', .lo = "ipv6", .clear = 0, .set = GOLB_IS6 }
} ;

int main (int argc, char const *const *argv)
{
  stralloc in = STRALLOC_ZERO ;
  unsigned int golc ;
  PROG = "tipidee-logaggregate" ;
  golc = gol_main(argc, argv, rgolb, 2, 0, 0, &wgolb, 0) ;
  argc -= golc ; argv += golc ;

  avltree_init(&pidip_map, 10, 3, 8, &pidip_dtok, &pid_cmp, &pidip_list) ;
  avltree_init(&ipinfo_map, 10, 3, 8, &ipinfo_dtok, &ip_cmp, &ipinfo_list) ;
  avltree_init(&storage_map, 10, 3, 8, &string_dtok, &string_cmp, &storage) ;

  for (;; line++, in.len = 0)
  {
    pid_t pid ;
    size_t pos = 0 ;
    size_t l ;
    char const *host = 0 ;
    int r = skagetln(buffer_0, &in, '\n') ;
    if (r == -1)
      if (errno != EPIPE) strerr_diefusys(111, "read from stdin") ;
      else break ;
    else if (!r) break ;
    if (!in.len || in.s[in.len - 1] != '\n') continue ;
    in.s[--in.len] = 0 ;
    if (in.s[pos] == '@')
    {
      if (in.len < 26) continue ;
      pos += 26 ;
    }
    if (pos + 15 > in.len) continue ;
    if (memcmp(in.s + pos, "tipideed: pid ", 14)) continue ;
    pos += 14 ;
    l = pid_scan(in.s + pos, &pid) ;
    if (!l) continue ;
    pos += l ;
    if (pos + 9 > in.len) continue ;
    if (memcmp(in.s + pos, ": info: ", 8))
    {
      if (!memcmp(in.s + pos, ": fatal: ", 9)) pidip_delete(pid) ;
      continue ;
    }
    pos += 8 ;
    if (pos + 5 > in.len) continue ;
    if (!memcmp(in.s + pos, "host ", 5))
    {
      pos += 5 ;
      host = in.s + pos ;
      l = byte_chr(in.s + pos, in.len - pos, ' ') ;
      if (pos + l >= in.len) continue ;
      pos += l ;
    }
    parse(pid, host, in.s + pos) ;
  } 

  {
    char fmt[UINT32_FMT] ;
    fmt[uint32_fmt(fmt, avltree_len(&ipinfo_map))] = 0 ;
    buffer_puts(buffer_1, "aggregated ") ;
    buffer_puts(buffer_1, fmt) ;
    buffer_putsflush(buffer_1, " ips\n") ;
  }
  (void)avltree_iter(&ipinfo_map, &print_iter, 0) ;
  _exit(0) ;
}
