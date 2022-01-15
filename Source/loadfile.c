/* loadfile.c
 * prototype code loading an entire file with a single read
 * and then converting to array of strings
 */

#ident "$Id: mylib loadfile.c default af8a97c423e5 2021-12-05 00:35:29Z rwmitchell $"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strcpy()
#include <getopt.h>
#include <sys/stat.h>   // fsize() / stat()
#include <fcntl.h>      // open()
#include <sys/types.h>  // read()
#include <sys/uio.h>    // read()
#include <unistd.h>     // read()
#include <sys/param.h>  // INT_MAX
#include <stdbool.h>
#include <sys/param.h>  // MIN/MAX
#include "io.h"
#include "bugout.h"
#include "utils.h"
#include "malloc.h"

extern int  debug;
// extern bool B_oride;     // 2020-12-06 globals in libs are bad

// ###############################################################
int RMfs_cnt( const char *buf, char fs ) {   // return number of FS in buf
  int   cnt = 0;
  const
  char *pb  = buf;
  while ( (pb = strchr( pb, fs)) ) { cnt++; pb++; };

  return( cnt );
}
int   RMcount_lines  ( const char  *fname ) {                 // count lines in file
  int rv = 0,
      ch;
  FILE *fp;

  if ( (fp=fopen(fname, "r")) == NULL ) {
    BUGERR("Unable to open: %s\n", fname );
    exit( __LINE__ );
  }

  while ( (ch=fgetc(fp)) != EOF ) if ( ch == '\n' ) ++rv;
  fclose( fp );

  return( rv );
}
int   RMmax_linelen( const char **arr ) {
  const
  char **pa = arr;
  size_t  max=0;
  while ( **pa ) {
//  STDOUT("%3lu: %s\n", strlen( *pa ), *pa );
    max = MAX( max, strlen( *pa ) );
    pa++;
  }
//STDOUT("%3lu: MANLEN\n", max );

  return( max );
}
int   RMstr2arr( char *mlstr, const char *FS, char ***arr, int lim ) {
  char *ch;
  int i, cnt, pos;

  if ( lim < 0 ) lim = INT_MAX;

  // count the number of FS in the string
  for (cnt=0, ch=mlstr; *ch != '\0' && cnt <= lim; ++ch )
    if ( strchr( FS, *ch ) != NULL ) cnt++;
  cnt++;

//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

  if ( cnt > lim ) cnt = lim;

  *arr = (char **) RMmalloc( *arr, sizeof(char *) * cnt );   // alloc space for array of pointers

  ch = mlstr;
  for (i=0, cnt=0, pos=0; *ch != '\0' && cnt < lim-1; ++i, ++ch ) {
    if ( strchr( FS, *ch ) != NULL ) {
      *ch = '\0';
      if ( i-pos > 1 ) {
//      BUGOUT("%2d:%d:%5d: >%s<\n", cnt, i, pos, &mlstr[pos] );
        (*arr)[cnt++] = &mlstr[pos];
      }
      pos = i+1;
    }
  }
  (*arr)[cnt++] = &mlstr[pos];   // save the last one
//BUGOUT("cnt: %d lim: %d\n", cnt, lim );

  return( cnt );
}
// ---------------------------------------------------------------
unsigned char *RMloadfile( const char *fname, off_t *f_sz, bool B_oride ) {
  off_t   f_limit = 384 * 2<<19;                 // ~ 384, arbitrary
  ssize_t rc = 0;
  int     fd;
  static
  unsigned char *data = NULL;

  if ( ! RMfile_exists( fname ) ) {
    BUGOUT("%s does not exist, exiting\n", fname );
    exit( __LINE__ );
  }

  *f_sz = RMfsize( fname )+0;                    // get file size
  if ( debug & 0x0001 )
    BUGOUT( "%s file size\n", RMoff_str( *f_sz ) );        // only portable way

  if ( *f_sz > 0 /* && *f_sz < f_limit */ ) {    // is file size in bounds?
    if ( *f_sz > f_limit ) {
      BUGOUT("%s: File size\n",  RMoff_str( *f_sz   ) );
      BUGOUT("%s: File limit\n", RMoff_str( f_limit ) );
    }
    if ( !B_oride ) *f_sz = MIN( *f_sz, f_limit );
    data = (unsigned char *) RMmalloc( data, *f_sz+1 );       // allocate space for file
//  memset(data, '\0', *f_sz+1 );

    if ( debug & 0x0001 )
    BUGOUT( "%p data: bytes: %s\n", data, RMoff_str( *f_sz ) );

    if ( (fd=open( fname, O_RDONLY )) > 0 ) {
      rc = read( fd, data, *f_sz );              // put entire file into data
      close( fd );
    } else {
      BUGERR( "Unable to open %s, %d\n", fname, fd );
    }
  } else {
    // Expect next line to fail as return string from RMoff_str is static
    BUGERR( "file size: %s   limit = %s\n", RMoff_str( *f_sz ), RMoff_str(  f_limit ) );  // size out of bounds
  }

  if ( rc != *f_sz )
    BUGOUT("Read %ld bytes of %s, expected %s\n", rc, fname, RMoff_str( *f_sz ) );

#ifdef  NOT_HELPFUL
  if ( debug & 0x0002 ) {
    BUGOUT( "File Contents\n" );
    STDOUT( "--------------\n" );
    STDOUT( "%s", data );
    STDOUT( "--------------\n" );
  }
#endif

  return( data );
}

// ###############################################################
#define BUF_SIZE 1024
#define BUF_MIN   256
unsigned char *RMloadstdin( off_t *f_sz ) {
  static
  char *input = NULL;
  char *pi;
  size_t sz;
  off_t  len = 0,
         size;

  if ( input ) RMfree( input );

  size   = BUF_SIZE;
  input  = (char *) RMmalloc( input, size );

  pi = input;
  while ( ! feof( stdin ) ) {
    if ( size-len <= BUF_MIN ) {
      size   *= 2;

#ifdef __APPLE__
      input = (char *) RMrealloc( input, size );
#else
      input = (char *) realloc ( input, size );
#endif
      if ( !input ) *f_sz = -1;
      pi = input + len;
    }

    sz = fread( (void *) (pi), 1, size-len, stdin );
    pi += sz;

    len += sz;
  }

  *f_sz = len;

  FILE *fp = freopen("/dev/tty", "r", stdin );
  if ( !fp ) BUGOUT( "%p: freopen( /dev/tty ) failed\n", fp )
//BUGOUT("LEN: %zd\n", len )
  return( (unsigned char *) input );
}

unsigned char *RMloadpipe( const char *cmd, off_t *f_sz ) {   // blatant copy of loadstdin()
  static
  char *input = NULL;
  char *pi;
  size_t sz;
  off_t  len = 0,
         size;
  FILE *pipe;

  if ( input ) free( input );

//BUGOUT( "%s: CMD\n", cmd  );

  if ( ( pipe = popen( cmd, "r" )) == NULL ) {
    BUGOUT( "%s: Unable to popen\n", cmd );
    return( NULL );
  }

  size   = BUF_SIZE;
  input  = (char *) RMmalloc( input, size );

  pi = input;
  while ( ! feof( pipe ) ) {
    if ( size-len <= BUF_MIN ) {
      size   *= 2;

      input = (char *) RMrealloc( input, size );
      if ( !input ) *f_sz = -1;
      pi = input + len;
    }

    sz = fread( (void *) (pi), 1, size-len, pipe );
    pi += sz;

    len += sz;
  }
//BUGOUT( "%lld: bytes read\n", len );

  *f_sz = len;

  pclose( pipe );
//BUGOUT("LEN: %lld\n", len )
  return( (unsigned char *) input );
}
