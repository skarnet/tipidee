/* ISC license. */

#include <stdint.h>
#include <string.h>

#include <skalibs/uint16.h>
#include <skalibs/bytestr.h>
#include <skalibs/fmtscan.h>
#include <skalibs/lolstdio.h>

#include <tipidee/uri.h>


/*

 Decodes an URI.
 Accepts absolute (http and https) and local, decodes %-encoding up to ? query.

st\ev	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e
	\0	invalid	%	?	/	:	@	h	t	p	s	0-9	a-f	other	delim

00					Pp
START	X	X	X	X	PATH?	X	X	H	X	X	X	X	X	X	X

01
H	X	X	X	X	X	X	X	X	HT	X	X	X	X	X	X

02
HT	X	X	X	X	X	X	X	X	HTT	X	X	X	X	X	X

03
HTT	X	X	X	X	X	X	X	X	X	HTTP	X	X	X	X	X

04						
HTTP	X	X	X	X	X	AUTH	X	X	X	X	HTTPS	X	X	X	X

05						s
HTTPS	X	X	X	X	X	AUTH	X	X	X	X	X	X	X	X	X

06
AUTH	X	X	X	X	AUTH/	X	X	X	X	X	X	X	X	X	X

07
AUTH/	X	X	X	X	HOST	X	X	X	X	X	X	X	X	X	X

08			H	Hp				Hp	Hp	Hp	Hp	Hp	Hp	Hp
HOST	X	X	HQ	H1	X	X	X	H1	H1	H1	H1	H1	H1	H1	X

09			p									a	a
HQ	X	X	H1	X	X	X	X	X	X	X	X	HQ1	HQ1	X	X

0a												ab	ab
HQ1	X	X	X	X	X	X	X	X	X	X	X	H1	H1	X	X

0b	p				zPp	zm		p	p	p	p	p	p	p
H1	END	X	HQ	H1	PATH	PORT	X	H1	H1	H1	H1	H1	H1	H1	X

0c					Pp							p
PORT	X	X	X	X	PATH	X	X	X	X	X	X	PORT1	X	X	X

0d	zc				zcPp							p
PORT1	END	X	X	X	PATH	X	X	X	X	X	X	PORT1	X	X	X

0e	p			zQ	p	p	p	p	p	p	p	p	p	p
PATH	END	X	Q	QUERY	PATH	PATH	PATH	PATH	PATH	PATH	PATH	PATH	PATH	PATH	X

0f			p									a	a
Q	X	X	PATH	X	X	X	X	X	X	X	X	Q1	Q1	X	X

10												ab	ab
Q1	X	X	X	X	X	X	X	X	X	X	X	PATH	PATH	X	X

11	p		p	p	p	p	p	p	p	p	p	p	p	p	p
QUERY	END	X	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY	QUERY

12	p			zQ		p	p	p	p	p	p	p	p	p
PATH?	END	X	Q	QUERY	X	PATH	PATH	PATH	PATH	PATH	PATH	PATH	PATH	PATH	X

st\ev	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e
	\0	invalid	%	?	/	:	@	h	t	p	s	0-9	a-f	other	delim

END = 13, X = 14

0x8000	s	ssl
0x4000	H	start host
0x2000	z	print \0
0x1000	m	mark
0x0800	c	scan port from mark, reset to mark
0x0400	P	start path
0x0200	p	print cur
0x0100	Q	start query
0x0080	a	push num
0x0040	b	decode num, print num, reinit

*/

static inline uint8_t uridecode_cclass (char c)
{
  static uint8_t const table[128] = "01111111111111111111111111111111161162>>>>>=>==4;;;;;;;;;;5>>=>36<<<<<<====================>1>==1<<<<<<=7=======9==:8======111=1" ;
  return c & 0x80 ? 1 : table[(uint8_t)c] - '0' ;
}

size_t tipidee_uri_parse (char *out, size_t max, char const *in, tipidee_uri *uri)
{
  static uint16_t const table[19][15] =
  {
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0612, 0x0014, 0x0014, 0x0001, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0002, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0003, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0004, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0006, 0x0014, 0x0014, 0x0014, 0x0014, 0x0005, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x8006, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0007, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0008, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x4009, 0x420b, 0x0014, 0x0014, 0x0014, 0x420b, 0x420b, 0x420b, 0x420b, 0x420b, 0x420b, 0x420b, 0x0014 },
    { 0x0014, 0x0014, 0x020b, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x008a, 0x008a, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x00cb, 0x00cb, 0x0014, 0x0014 },
    { 0x0213, 0x0014, 0x0009, 0x000b, 0x260e, 0x300c, 0x0014, 0x020b, 0x020b, 0x020b, 0x020b, 0x020b, 0x020b, 0x020b, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x060e, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x020d, 0x0014, 0x0014, 0x0014 },
    { 0x2813, 0x0014, 0x0014, 0x0014, 0x2e0e, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x020d, 0x0014, 0x0014, 0x0014 },
    { 0x0213, 0x0014, 0x000f, 0x2111, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x0014 },
    { 0x0014, 0x0014, 0x020e, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0090, 0x0090, 0x0014, 0x0014 },
    { 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x00ce, 0x00ce, 0x0014, 0x0014 },
    { 0x0213, 0x0014, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211, 0x0211 },
    { 0x0213, 0x0014, 0x000f, 0x2111, 0x0014, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x020e, 0x0014 }
  } ;
  size_t w = 0, lastslash = 0, mark = 0 ;
  char const *host = 0 ;
  char const *path = 0 ;
  char const *query = 0 ;
  uint16_t port = 0 ;
  uint16_t state = 0 ;
  unsigned char decoded = 0 ;
  uint8_t ssl = 0 ;
  for (; state < 0x13 ; in++)
  {
    uint16_t c = table[state][uridecode_cclass(*in)] ;
/*
    LOLDEBUG("tipidee_uri_parse: state %hu, event %c, newstate %hu, actions %s%s%s%s%s%s%s%s%s%s", state, *in, c & 0x1f,
      c & 0x8000 ? "s" : "",
      c & 0x4000 ? "H" : "",
      c & 0x2000 ? "z" : "",
      c & 0x1000 ? "m" : "",
      c & 0x0800 ? "c" : "",
      c & 0x0400 ? "P" : "",
      c & 0x0200 ? "p" : "",
      c & 0x0100 ? "Q" : "",
      c & 0x0080 ? "a" : "",
      c & 0x0040 ? "b" : ""
    ) ;
*/
    state = c & 0x1f ;
    if (c & 0x8000) ssl = 1 ;
    if (c & 0x4000) host = out + w ;
    if (c & 0x2000) { if (w >= max) return 0 ; out[w++] = 0 ; }
    if (c & 0x1000) mark = w ;
    if (c & 0x0800) { if (!uint160_scan(out + mark, &port) || !port) return 0 ; w = mark ; }
    if (c & 0x0400) path = out + w ;
    if (c & 0x0200) { if (w >= max) return 0 ; out[w++] = *in ; }
    if (c & 0x0100) query = out + w ;
    if (c & 0x0080) decoded = (decoded << 4) | fmtscan_num(*in, 16) ;
    if (c & 0x0040)
    {
      if (w >= max) return 0 ;
      if (decoded == '/') lastslash = w ;
      out[w++] = decoded ;
      decoded = 0 ;
    }
  }
  if (state > 0x13) return 0 ;
  if (path)
  {
    size_t len = strlen(path) ;
    if (len >= 3 && !memcmp(path + len - 3, "/..", 3)) return 0 ;
    if (strstr(path, "/../")) return 0 ;
  }
  uri->host = host ;
  uri->port = port ;
  uri->path = path ? path : "/" ;
  uri->query = query ;
  uri->lastslash = path ? lastslash - (path - out) : 0 ;
  uri->https = ssl ;
  return w ;
}
