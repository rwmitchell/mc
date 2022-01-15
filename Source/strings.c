#include <stdio.h>
#include <string.h>
#include <ctype.h>   // isspace()
#include "bugout.h"

char *RMadjustl( char *str, int len ) {     // move leading whitespace to the right side
  char *hd,  // head
       *cp;  // current position
  int  i,
       cnt = 0;

  hd = cp = str;
  while ( isspace( *str ) && cnt++ < len ) str++; // skip whitespace
  for (i=cnt; i<len; ++i ) *(cp++) = *(str++);    // copy
  for (i=0; i<cnt; ++i) *(cp++) = ' ';            // append space
  return( hd );
}
char *RMrtrim( char *str, int len ) {       // trim whitespace from right side
  char *cp;  // current position

  int cnt = 0;

  --len;
  cp = str+len;

  while ( isspace( *cp ) && cnt++ < len ) cp--;   // skip whitespace
  *(++cp) = '\0';

  printf("%s:%d: cnt = %d\n", __func__, __LINE__, cnt );
  return( str );
}
// sorta copied from basename.c but doesn't overwrite in
char *RMtruncname( char *in, char *out, char sep ) {
  char *period, *cin, *cout;

  period = strrchr( in, sep );
  if ( !period ) {
    strcpy( out, in );
  } else {
    for (cin=in, cout= out; cin != period; ++cin, ++cout ) *cout = *cin;
    *cout = '\0';
  }
  return( out );
}

char *RMlower( char *str ) {
  char *ps = str;
  while ( *ps != '\0' ) { *ps = tolower( *ps ); ++ps; }
  return( str );
}
#ifdef __linux__
char *RMstrcasestr( const char *haystack, const char *needle ) {
  char *h = strdup( haystack ),
       *n = strdup( needle   ),
       *rv= NULL;

  h = RMlower( h );
  n = RMlower( n );
  char *p = strstr( h, n );   // find string using lowercase versions

  STDOUT( "h: <%s>\n", h );
  STDOUT( "n: <%s>\n", n );
  // find position in haystack using p - h offset
  if ( p ) rv = ( char * ) ( haystack + ( p-h ) );

  STDOUT( "offset: %ld\n",  p-h );
  STDOUT( "p: >%s<\n", p );
  STDOUT( "r: >%s<\n", rv);

  return( rv );
}
#else
char *RMstrcasestr( const char *haystack, const char *needle ) {
  return( strcasestr( haystack, needle ));
}
#endif
