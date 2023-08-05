/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/error.h>
#include <skalibs/avltreen.h>
#include <skalibs/unix-timed.h>
// #include <skalibs/lolstdio.h>

#include <tipidee/headers.h>

/*

Reads header lines, separates into \0-terminated keys+values.
key is at hdr->buf + hdr->list[i].left
value is at hdr->buf + hdr->list[i].right 
Compresses linear whitespace.
Does not unquote strings/comments in values. 


st\ev	0	1	2	3	4	5	6	7
	CTL	CR	LF	LWS	:	special normal	8bit

00							kp
START	X	END?	END	X	X	X	K	X

01
END?	X	X	END	X	X	X	X	X

02					zv		p
K	X	X	X	X	V1	X	K	X

03					p	p	p	p
V1	X	V1??	V1?	V1	V	V	V	V

04
V1??	X	X	V1?	X	X	X	X	X

05		zn	zn				znkp
V1?	X	END?	END	V1	X	X	K	X

06		s	s	s	p	p	p	p
V	X	V2??	V2?	V2	V	V	V	V

07					p	p	p	p
V2	X	V2??	V2?	V2	V	V	V	V

08
V2??	X	X	V2?	X	X	X	X	X

09		mzn	mzn				mznkp
V2?	X	END?	END	V2	X	X	K	X

END = 0a, X = 0b

0x4000 s: write space
0x2000 m: go back one char
0x1000 z: write \0
0x0800 n: cut key/value pair, prepare next
0x0400 k: start of key
0x0200 v: start of value
0x0100 p: write current char

states: 4 bits, actions: 7 bits

*/


struct tainp_s
{
  tain const *deadline ;
  tain *stamp ;
} ;

typedef int get1_func (buffer *, char *, struct tainp_s *) ;
typedef get1_func *get1_func_ref ;

static int get1_timed (buffer *b, char *c, struct tainp_s *d)
{
  return buffer_timed_get(b, c, 1, d->deadline, d->stamp) ;
}

static int get1_notimed (buffer *b, char *c, struct tainp_s *data)
{
  (void)data ;
  return buffer_get(b, c, 1) == 1 ;
}

static uint8_t cclass (char c)
{
  static uint8_t const ctable[128] = "00000000032001000000000000000000365666665566566566666666664565655666666666666666666666666665556666666666666666666666666666656560" ;
  return c & 0x80 ? 7 : ctable[(uint8_t)c] - '0' ;
}

static int needs_processing (char const *s)
{
  if (!strcasecmp(s, "Set-Cookie")) return 0 ;
  if (str_start(s, "X-")) return 0 ;
  return 1 ;
}

static int tipidee_headers_parse_with (buffer *b, tipidee_headers *hdr, get1_func_ref next, struct tainp_s *data, disize *header, uint32_t *state)
{
  static uint16_t const table[10][8] =
  {
    { 0x000b, 0x0001, 0x000a, 0x000b, 0x000b, 0x000b, 0x0502, 0x000b },
    { 0x000b, 0x000b, 0x000a, 0x000b, 0x000b, 0x000b, 0x000b, 0x000b },
    { 0x000b, 0x000b, 0x000b, 0x000b, 0x1203, 0x000b, 0x0102, 0x000b },
    { 0x000b, 0x0004, 0x0005, 0x0003, 0x0106, 0x0106, 0x0106, 0x0106 },
    { 0x000b, 0x000b, 0x0005, 0x000b, 0x000b, 0x000b, 0x000b, 0x000b },
    { 0x000b, 0x1801, 0x180a, 0x0003, 0x000b, 0x000b, 0x1d02, 0x000b },
    { 0x000b, 0x4008, 0x4009, 0x4007, 0x0106, 0x0106, 0x0106, 0x0106 },
    { 0x000b, 0x0008, 0x0009, 0x0007, 0x0106, 0x0106, 0x0106, 0x0106 },
    { 0x000b, 0x000b, 0x0009, 0x000b, 0x000b, 0x000b, 0x000b, 0x000b },
    { 0x000b, 0x3801, 0x380a, 0x0007, 0x000b, 0x000b, 0x3d02, 0x000b },
  } ;
  while (*state < 0x0a)
  {
    uint16_t c ;
    char cur ;
    if (!(*next)(b, &cur, data))
      return errno == ETIMEDOUT ? 408 : error_isagain(errno) ? -2 : -1 ;
    c = table[*state][cclass(cur)] ;
/*
   {

      char s[2] = { cur, 0 } ;
      LOLDEBUG("tipidee_headers_parse_with: state %hhu, event %s, newstate %hhu, actions %s%s%s%s%s%s%s",
        *state,
        cur == '\r' ? "\\r" : cur == '\n' ? "\\n" : s,
        c & 0x0f,
        c & 0x4000 ? "s" : "",
        c & 0x2000 ? "m" : "",
        c & 0x1000 ? "z" : "",
        c & 0x0800 ? "n" : "",
        c & 0x0400 ? "k" : "",
        c & 0x0200 ? "v" : "",
        c & 0x0100 ? "p" : ""
      ) ;
    }
*/
    *state = c & 0x0f ;
    if (c & 0x4000) { if (hdr->len >= hdr->max) return 413 ; hdr->buf[hdr->len++] = ' ' ; }
    if (c & 0x2000) hdr->len-- ;
    if (c & 0x1000) { if (hdr->len >= hdr->max) return 413 ; hdr->buf[hdr->len++] = 0 ; }
    if (c & 0x0800)
    {
      uint32_t prev ;
      if (hdr->n >= TIPIDEE_HEADERS_MAX) return 413 ;
      hdr->list[hdr->n] = *header ;
      if (needs_processing(hdr->buf + header->left))
      {
//      LOLDEBUG("tipidee_headers_parse_with: n: adding header %u - key %zu (%s), value %zu (%s)", hdr->n, header->left, hdr->buf + header->left, header->right, hdr->buf + header->right) ;
        if (avltreen_search(&hdr->map, hdr->buf + header->left, &prev))
        {
          size_t start = hdr->list[prev+1].left ;
          if (prev+1 == hdr->n)
          {
            hdr->buf[start - 1] = ',' ;
            hdr->buf[start] = ' ' ;
            memcpy(hdr->buf + start + 1, hdr->buf + header->right, hdr->len - header->right) ;
          }
          else
          {
            size_t len = header->left - start ;
            size_t offset = hdr->len - header->right + 1 ;
            char tmp[len] ;
            memcpy(tmp, hdr->buf + start, len) ;
            hdr->buf[start - 1] = ',' ;
            hdr->buf[start] = ' ' ;
            memcpy(hdr->buf + start + 1, hdr->buf + header->right, hdr->len - header->right) ;
            memcpy(hdr->buf + start + offset, tmp, len) ;
            for (uint32_t i = prev + 1 ; i < hdr->n ; i++)
            {
              hdr->list[i].left += offset ;
              hdr->list[i].right += offset ;
            }
          }
          hdr->len -= header->right - header->left - 1 ;
          hdr->n-- ;
        }
        else if (!avltreen_insert(&hdr->map, hdr->n)) return 500 ;
      }
      hdr->n++ ;
    }
    if (c & 0x0400) header->left = hdr->len ;
    if (c & 0x0200) header->right = hdr->len ;
    if (c & 0x0100) { if (hdr->len >= hdr->max) return 413 ; hdr->buf[hdr->len++] = cur ; }
  }
  if (*state > 0x0a) return 400 ;
  return 0 ;
}

int tipidee_headers_timed_parse (buffer *b, tipidee_headers *hdr, tain const *deadline, tain *stamp)
{
  struct tainp_s d = { .deadline = deadline, .stamp = stamp } ;
  disize header = DISIZE_ZERO ;
  uint32_t state = 0 ;
  return tipidee_headers_parse_with(b, hdr, &get1_timed, &d, &header, &state) ;
}

int tipidee_headers_parse_nb (buffer *b, tipidee_headers *hdr, disize *header, uint32_t *state)
{
  return tipidee_headers_parse_with(b, hdr, &get1_notimed, 0, header, state) ;
}
