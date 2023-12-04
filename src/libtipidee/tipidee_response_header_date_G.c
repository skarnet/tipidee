/* ISC license. */

#include <skalibs/tai.h>

#include <tipidee/response.h>

size_t tipidee_response_header_date_G (char *s, size_t max)
{
  tain wstamp ;
  tain_wallclock_read(&wstamp) ;
  return tipidee_response_header_date(s, max, &wstamp) ;
}
