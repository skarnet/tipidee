/* ISC license. */

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <skalibs/uint32.h>
#include <skalibs/sgetopt.h>
#include <skalibs/buffer.h>
#include <skalibs/strerr.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/direntry.h>
#include <skalibs/djbunix.h>
#include <skalibs/skamisc.h>
#include <skalibs/avltree.h>

#define USAGE "tipidee-config-preprocess file"
#define dieusage() strerr_dieusage(100, USAGE)
#define dienomem() strerr_diefu1sys(111, "build internal strings") ;

static stralloc sa = STRALLOC_ZERO ;
static genalloc ga = GENALLOC_ZERO ;  /* size_t */


 /* Name storage */

static stralloc namesa = STRALLOC_ZERO ;

static void *name_dtok (uint32_t pos, void *aux)
{
  return ((stralloc *)aux)->s + pos + 1 ;
}

static int name_cmp (void const *a, void const *b, void *aux)
{
  (void)aux ;
  return strcmp((char const *)a, (char const *)b) ;
}

static avltree namemap = AVLTREE_INIT(8, 3, 8, &name_dtok, &name_cmp, &namesa) ;


 /* Directory sorting */

static char const *dname_cmp_base ;
static int dname_cmp (void const *a, void const *b)
{
  return strcmp(dname_cmp_base + *(size_t *)a, dname_cmp_base + *(size_t *)b) ;
}


 /* Recursive inclusion functions */

static void includefromhere (char const *) ;

static inline void includecwd (void)
{
  DIR *dir ;
  size_t sabase = sa.len ;
  size_t gabase = genalloc_len(size_t, &ga) ;
  if (sagetcwd(&sa) == -1) strerr_diefu1sys(111, "getcwd") ;
  if (!stralloc_0(&sa)) dienomem() ;
  dir = opendir(".") ;
  if (!dir) strerr_diefu2sys(111, "opendir ", sa.s + sabase) ;

  for (;;)
  {
    direntry *d ;
    errno = 0 ;
    d = readdir(dir) ;
    if (!d) break ;
    if (d->d_name[0] == '.') continue ;
    if (!genalloc_catb(size_t, &ga, &sa.len, 1)) dienomem() ;
    if (!stralloc_catb(&sa, d->d_name, strlen(d->d_name)+1)) dienomem() ;
  }
  dir_close(dir) ;
  if (errno) strerr_diefu2sys(111, "readdir ", sa.s + sabase) ;

  dname_cmp_base = sa.s ;
  qsort(genalloc_s(size_t, &ga) + gabase, genalloc_len(size_t, &ga) - gabase, sizeof(size_t), &dname_cmp) ;

  for (size_t i = 0 ; i < genalloc_len(size_t, &ga) ; i++)
    includefromhere(sa.s + genalloc_s(size_t, &ga)[gabase + i]) ;

  genalloc_setlen(size_t, &ga, gabase) ;
  sa.len = sabase ;
}

static void include (char const *file)
{
  size_t sabase = sa.len ;
  size_t filelen = strlen(file) ;
  if (!sadirname(&sa, file, filelen) || !stralloc_0(&sa)) dienomem() ;
  if (chdir(sa.s + sabase) == -1) strerr_diefu2sys(111, "chdir to ", sa.s + sabase) ;
  sa.len = sabase ;
  if (!sabasename(&sa, file, filelen)) dienomem() ;
  {
    char fn[sa.len + 1 - sabase] ;
    memcpy(fn, sa.s + sabase, sa.len - sabase) ;
    fn[sa.len - sabase] = 0 ;
    sa.len = sabase ;
    includefromhere(fn) ;
  }
}

static inline int idcmd (char const *s)
{
  static char const *commands[] =
  {
    "include",
    "includedir",
    "included:",
    0
  } ;
  for (char const **p = commands ; *p ; p++)
    if (!strcmp(s, *p)) return p - commands ;
  return -1 ;
}

static inline unsigned char cclass (char c)
{
  static unsigned char const classtable[34] = "0444444443144344444444444444444432" ;
  return (unsigned char)c < 34 ? classtable[(unsigned char)c] - '0' : 4 ;
}

static void includefromhere (char const *file)
{
  static unsigned char const table[8][5] =
  {
    { 0x08, 0x10, 0x02, 0x11, 0x11 },
    { 0x08, 0x10, 0x11, 0x11, 0x11 },
    { 0x08, 0x00, 0x03, 0x04, 0x25 },
    { 0x08, 0x00, 0x03, 0x03, 0x03 },
    { 0x09, 0x09, 0x09, 0x04, 0x25 },
    { 0x09, 0x09, 0x09, 0x46, 0x25 },
    { 0x0a, 0x0a, 0x07, 0x06, 0x27 },
    { 0x88, 0x80, 0x27, 0x27, 0x27 }
  } ;
  size_t sabase = sa.len ;
  size_t namesabase = namesa.len ;
  size_t sastart ;
  int cmd = -1 ;
  int fd ;
  buffer b ;
  uint32_t d ;
  uint32_t line = 1 ;
  char buf[4096] ;
  unsigned char state = 0 ;

  if (!stralloc_catb(&namesa, "\004", 1)) dienomem() ;
  if (sarealpath(&namesa, file) == -1)
  {
    cmd = errno ;
    if (sagetcwd(&sa) == -1) strerr_diefu1sys(111, "getcwd") ;
    if (!stralloc_0(&sa)) dienomem() ;
    errno = cmd ;
    strerr_dief4sys(111, "from directory ", sa.s + sabase, ": unable to realpath ", file) ;
  }
  if (!stralloc_0(&namesa)) dienomem() ;
  if (avltree_search(&namemap, namesa.s + namesabase + 1, &d))
  {
    if (namesa.s[d] & 0x04)
      strerr_dief3x(2, "file ", namesa.s + namesabase + 1, " is included in a cycle") ;
    if (!(namesa.s[d] & 0x02))
      strerr_dief3x(2, "file ", namesa.s + namesabase + 1, " is included twice but does not declare !included: unique or !included: multiple") ;
    namesa.len = namesabase ;
    if (namesa.s[d] & 0x01) return ;
  }
  else
  {
    if (namesabase > UINT32_MAX)
      strerr_dief3x(100, "in ", namesa.s + d + 1, ": too many, too long filenames") ;
    d = namesabase ;
    if (!avltree_insert(&namemap, d)) dienomem() ;
  }

  if (!string_quote_nospace(&sa, namesa.s + d + 1, strlen(namesa.s + d + 1))) dienomem() ;
  if (!stralloc_0(&sa)) dienomem() ;

  sastart = sa.len ;
  fd = open_readb(file) ;
  if (fd == -1) strerr_diefu2sys(111, "open ", namesa.s + d + 1) ;
  buffer_init(&b, &buffer_read, fd, buf, 4096) ;

  if (buffer_put(buffer_1, "! 0 ", 4) < 4
   || buffer_puts(buffer_1, sa.s + sabase) < 0
   || buffer_put(buffer_1, "\n", 1) < 1)
    strerr_diefu1sys(111, "write to stdout") ;

  while (state < 8)
  {
    uint16_t what ;
    char c = 0 ;
    if (buffer_get(&b, &c, 1) < 0) strerr_diefu2sys(111, "read from ", namesa.s + d + 1) ;
    what = table[state][cclass((unsigned char)c)] ;
    state = what & 0x000f ;
    if (what & 0x0010) if (buffer_put(buffer_1, &c, 1) < 1) strerr_diefu1sys(111, "write to stdout") ;
    if (what & 0x0020) if (!stralloc_catb(&sa, &c, 1)) dienomem() ;
    if (what & 0x0040)
    {
      if (!stralloc_0(&sa)) dienomem() ;
      cmd = idcmd(sa.s + sastart) ;
      if (cmd == -1)
      {
        char linefmt[UINT32_FMT] ;
        linefmt[uint32_fmt(linefmt, line)] = 0 ;
        strerr_dief6x(1, "in ", namesa.s + d + 1, " line ", linefmt, ": unrecognized directive: ", sa.s + sastart) ;
      }
      sa.len = sastart ;
    }
    if (what & 0x0080)
    {
      if (!stralloc_0(&sa)) dienomem() ;
      switch (cmd)
      {
        case 2 :
          if (namesa.s[d] & 2)
          {
            char linefmt[UINT32_FMT] ;
            linefmt[uint32_fmt(linefmt, line)] = 0 ;
            strerr_dief5x(1, "in ", namesa.s + d + 1, " line ", linefmt, ": extra !included: directive") ;
          }
          if (!strcmp(sa.s + sastart, "unique")) namesa.s[d] |= 3 ;
          else if (!strcmp(sa.s + sastart, "multiple")) namesa.s[d] |= 2 ;
          else
          {
            char linefmt[UINT32_FMT] ;
            linefmt[uint32_fmt(linefmt, line)] = 0 ;
            strerr_dief6x(1, "in ", namesa.s + d + 1, " line ", linefmt, ": invalid !included: argument: ", sa.s + sastart) ;
          }
          break ;
        case 1 :
        case 0 :
        {
          int fdhere = open2(".", O_RDONLY | O_DIRECTORY) ;
          char linefmt[UINT32_FMT] ;
          if (fdhere == -1)
            strerr_dief3sys(111, "in ", namesa.s + d + 1, ": unable to open base directory: ") ;
          linefmt[uint32_fmt(linefmt, line)] = 0 ;
          if (cmd & 1)
          {
            if (chdir(sa.s + sastart) == -1)
              strerr_dief6sys(111, "in ", namesa.s + d + 1, " line ", linefmt, ": unable to chdir to ", sa.s + sastart) ;
            includecwd() ;
          }
          else include(sa.s + sastart) ;
          if (fchdir(fdhere) == -1)
            strerr_dief4sys(111, "in ", namesa.s + d + 1, ": unable to fchdir back after including ", sa.s + sastart) ;
          fd_close(fdhere) ;
          if (buffer_put(buffer_1, "! ", 2) < 2
           || buffer_puts(buffer_1, linefmt) < 0
           || buffer_put(buffer_1, " ", 1) < 1
           || buffer_puts(buffer_1, sa.s + sabase) < 0
           || buffer_put(buffer_1, "\n", 1) < 1)
            strerr_diefu1sys(111, "write to stdout") ;
          break ;
        }
      }
      sa.len = sastart ;
    }
    if (c == '\n' && state <= 8) line++ ;
  }
  if (state > 8)
  {
    char linefmt[UINT32_FMT] ;
    linefmt[uint32_fmt(linefmt, line)] = 0 ;
    strerr_dief5x(1, "in ", namesa.s + d + 1, " line ", linefmt, ": syntax error: invalid ! line") ;
  }
  fd_close(fd) ;
  sa.len = sabase ;
  namesa.s[d] &= ~0x04 ;
}

int main (int argc, char const *const *argv, char const *const *envp)
{
  PROG = "tipidee-config-preprocess" ;
  {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, argv, "", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
        default : dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  if (!argc) dieusage() ;

  include(argv[0]) ;
  if (!buffer_flush(buffer_1))
    strerr_diefu1sys(111, "write to stdout") ;
  return 0 ;
}
