#include <stdio.h>
#include <stdlib.h>
#include <string.h>    // strcpy()
#include <getopt.h>
#include <ctype.h>     // isdigit()
#include <stdbool.h>   // bool
#include <sys/param.h> // MIN MAX
#include <wchar.h>
#include "malloc.h"
#include "loadfile.h"
#include "helpd.h"
#include "bugout.h"
// #include <mylib.h>

const
char *gitid = "$Id: 1d1e327ab79af1f29dbdb02fe5be7acb6fe4d828 $",
     *myid  = "$MyId: prism 2022-05-08 18:33:13 -0400  (HEAD -> main) 00b6e54 $",
     *date  = "$Date: 2022-05-08 18:33:13 -0400 $",
     *source= "$Source: /Users/rwmitchell/git/GitHub/prism $",
     *auth  = "$Auth: Richard Mitchell <rwmitchell@gmail.com> $";

const
char *cvsid = "$Id: mytools mc.c default e9eca572cf25 2021-12-26 15:43:30Z rwmitchell $";

const char *TF[]= {"False", "True"};

char myarg[1024],   // temporary optarg value
     myopt[1024];   // example optional argument
int  debug =  0,
     mrow  =  5,
     wdth  = 80,
     wbuf  =  3;    // white spacing between columns

bool B_strip = false,   // strip and columnize
     B_onlys = false,   // strip only
     B_big   = false,   // only columnize if input > screen rows
     B_colmn = true,
     _itwasme;

int cleanbuf      ( char *buf ) {
  char *pbuf   = buf,
       *nbuf   = buf;

  int state = 0,
      olen  = 0,
      nlen  = 0;

  while ( *pbuf ) {
    olen++;
    if ( *pbuf == '\033' ) {   // Escape Sequence starts
      state = 1;
    } else if ( state == 1 ) {
      if (( 'a' <= *pbuf && *pbuf <= 'z' ) || ( 'A' <= *pbuf && *pbuf <= 'Z' ))
        state = 2;
    } else {
      state = 0;
    }

    if ( !state ) {
      *nbuf = *pbuf;
      nbuf++;
      nlen++;
    }
    pbuf++;
  }
  *nbuf = '\0';

  return( nlen );
}
int *line_lengths ( char *buf ) {     // uses cleanbuf() to prefilter escape codes
  char *pc, *pl,
       nl = '\n';
  int *arr = NULL,
       i=0,
       nl_cnt = RMfs_cnt( buf, nl );

  char *tmp = RMstrdup ( buf );
  cleanbuf( tmp );

  arr = (int *) RMmalloc( arr, sizeof( int ) * nl_cnt );
  pc  = pl = tmp;

  int nn = 0;
  while ( ( pc = strchr( pc, nl ) ) ) {
    nn = 0;
    for (long j=0; j<(pc-pl); ++j ) if ( (pl[j]&0377) > 0200 ) nn++;
    arr[i++] = pc-pl-nn;
    pl = ++pc;
  }
  RMfree( tmp );

  return( arr );
}
int *line_lengths2( char *buf ) {     // calculate without pre-filtering
  char *pbuf   = buf;

  char nl = '\n';
  int *arr = NULL,
       i=0,
       nl_cnt = RMfs_cnt( buf, nl );

  int state = 0,
      len   = 0,
      wlen  = 0;

  arr = (int *) RMmalloc( arr, sizeof( int ) * nl_cnt );

  while ( *pbuf ) {
    if ( *pbuf == '\033' ) {   // Escape Sequence starts
      state = 1;
    } else if ( state == 1 ) {
      if (( 'a' <= *pbuf && *pbuf <= 'z' ) || ( 'A' <= *pbuf && *pbuf <= 'Z' ))
        state = 2;
    } else {
      state = 0;
    }

    if ( !state ) {
      if ( (*pbuf & 0377) < 0200 ) len++;      // count normal chars
      else                         wlen++;     // count wide   chars as 1/2
    }

    if ( *pbuf == nl ) {
      arr[i++] = (len + (wlen/2 ) ) -1;        // don't count newline
      len = wlen = 0;
    }
    pbuf++;
  }
//BUGOUT( "%d: MaxLen\n", max );

  return( arr );
}
int maxprintlen   ( char *buf ) {
  char *pbuf   = buf;

  int state = 0,
      max   = 0,
      len   = 0;

  while ( *pbuf ) {
    if ( *pbuf == '\033' ) {   // Escape Sequence starts
      state = 1;
    } else if ( state == 1 ) {
      if (( 'a' <= *pbuf && *pbuf <= 'z' ) || ( 'A' <= *pbuf && *pbuf <= 'Z' ))
        state = 2;
    } else {
      state = 0;
    }

    if ( !state ) len++;
    if ( *pbuf == '\n' ) {
      max = MAX( max, len-1 );   // don't count newline
      len = 0;
    }
    pbuf++;
  }
//BUGOUT( "%d: MaxLen\n", max );

  return( max );
}
void one_line     ( const char *progname ) {
  STDOUT("%-20s: print STDIN in multiple columns\n", progname );
  exit(0);
}
void help         ( char *progname, const char *opt, struct option lopts[] ) {

  STDERR("%s %s\n", __DATE__, __TIME__ );
  STDERR("%s\n%s\n\n", gitid, source );
  STDERR("usage: %s [-%s] [FILE]\n", progname, opt);
  STDERR("Print stdin in multiple columns\n");
  STDERR("\n");
  STDERR("  -b : (%-5s) Only columnate to avoid scrolling\n", TF[B_big  ] );
  STDERR("  -s : (%-5s) Strip escape codes from stdin\n",     TF[B_strip] );
  STDERR("  -S : (%-5s) Strip and print\n",                   TF[B_onlys] );
  STDERR("  -w   (%5d) Override COLUMNS\n",                   wdth        );
  STDERR("  -m   (%5d) Set minimum lines to split\n",         mrow        );
  STDERR("  -c   (%5d) Set spacing between columns\n",        wbuf        );
  STDERR("  -d  [0x%04x] Debug level\n", debug );
  STDERR("               use 0x0200 to invoke malloc_debug\n");
  STDERR("\n");
  STDERR("For better results:\n");
  STDERR("  export COLUMNS LINES\n");
  STDERR("  export MC_MIN  # set minimum lines to split\n");
  STDERR("\n");

  if ( debug ) RMhelpd( lopts );

  exit(-0);
}

int main(int argc, char *argv[]) {
  int errflg = 0,
      dinc   = 1,                // debug incrementor
      opt,
      longindex=0;
  bool B_have_arg = true;
  extern int   optind,
               optopt;
  extern char *optarg;

  const
  char *opts=":bc:m:sd:Sw:uh1";      // Leading : makes all :'s optional
  static struct option longopts[] = {
    { "big",             no_argument, NULL, 'b' },  // use columns if input > term rows
    { "minimum",   required_argument, NULL, 'm' },  // minimum lines to split
    { "colbuf",    required_argument, NULL, 'c' }, // Spacing between columns
    { "strip",           no_argument, NULL, 's' },
    { "debug",     optional_argument, NULL, 'd' },
    { "onlystrip",       no_argument, NULL, 'S' },
    { "width",     required_argument, NULL, 'w' },  // override COLUMNS
    { "version",         no_argument, NULL, 201 },
    { "help",            no_argument, NULL, 'h' },
    { "usage",           no_argument, NULL, 'u' },
    { "oneline",         no_argument, NULL, '1' },
    { NULL,                        0, NULL,  0  }
  };

  strcpy(myopt, "defval");

  char *TERM_columns = getenv( "COLUMNS" ),
       *TERM_lines   = getenv( "LINES"   ),
       *MC_min       = getenv( "MC_MIN"  );
  int   hght    = 24;

  if (  TERM_columns ) wdth = strtol( TERM_columns, NULL, 10 );
  if (  TERM_lines   ) hght = strtol( TERM_lines,   NULL, 10 );
  if (  MC_min       ) mrow = strtol( MC_min,       NULL, 10 );

//BUGOUT( "%d: screen width\n", wdth );

  // parse command line options
  while ( ( opt=getopt_long_only(argc, argv, opts, longopts, &longindex )) != EOF ) {

    B_have_arg = true;
    memset( myarg, '\0', 1024 );      // reset

    if ( opt == 0 ) {                 // we got a longopt
      opt = longopts[longindex].val;  // set opt to short opt value
//  } else {
//    BUGOUT("shortopt: %c:%d (%s)\n", opt, opt, optarg );
    }

    // Pre-Check
    if ( optarg ) {                // only check if not null
      switch (opt) {               // check only args with possible STRING options
        case 'o':
        case 'd':
          if ( *optarg == '\0' ) {
            BUGOUT("optarg is empty\n");
            if ( argv[optind] == NULL ) {
              BUGOUT("next arg is also NULL\n");
              B_have_arg = false;
            } else {
              BUGOUT("next arg is %d:%s\n", optind, argv[optind] );
              strcpy(myarg, argv[optind++]);
            }
          } else if ( *optarg == '-' ) {  // optarg is actually the next option
            BUGOUT("optarg is: %s, probably next option\n", optarg);
            --optind;
            B_have_arg = false;
          } else {
            BUGOUT("optional arg for %s is %s\n", longopts[longindex].name, optarg );

            strcpy(myarg, optarg);
            BUGOUT("optarg = %c(%s)\n", *optarg, myarg);
          }
          break;
      }
    } else
      B_have_arg = false;          // optarg was null

    // Normal Check
    switch (opt) {
      case ':':              // check optopt for previous option
//      BUGOUT("Got a Colon for: %c\n", optopt );
//      B_have_arg = false;        // not read?  ( gcc --analyze -I ~/Build/include Source/mc.c )
        switch( optopt ) {
          case 'd': debug += dinc; /* BUGOUT("debug level: %d\n", debug ); */ dinc <<= 1; break;
          default : BUGOUT("No arg for %c\n", optopt ); break;
        }
        break;

      case 'b': B_big   = !B_big  ; break;
      case 's': B_strip = !B_strip; break;
      case 'S': B_onlys = !B_onlys; break;

      case 'm': mrow    = strtol( optarg, NULL, 10 ); break;
      case 'w': wdth    = strtol( optarg, NULL, 10 ); break;
      case 'c': wbuf    = strtol( optarg, NULL, 10 ); break;

      case 201:
                STDOUT( "%s\n", gitid       );
                STDOUT( "%s\n", myid        );
                STDOUT( "%s\n", source      );
                STDOUT( "%s\n", date        );
                STDOUT( "%s\n", auth        );
                exit(0);
                break;

      case 'd':                      // set debug level
        if ( B_have_arg ) {
          debug |= strtol(myarg, NULL, 16 );
          if ( debug == 0 ) {        // we didn't get a number
            debug = 1;               // we could verify by checking errno, but why?
            --optind;
          }
        } else {
          BUGOUT("increasing debug(%d) by %d\n", debug, dinc );
          debug += dinc; dinc <<= 1;
        }
        BUGOUT("debug level: 0x%02X : %02X\n", debug, dinc );
        break;

      case 'u': // output opts with spaces
        RMusage( longopts );
        break;

      case '1': one_line( argv[0] ); break;

      case 'h':
      default :
        ++errflg;
        break;
    }
  }

  if (errflg) help(argv[0], opts, longopts);

  off_t  f_sz = 0;
  int    l_cnt,
         min_len,
         max_len = 0,
         avg_len = 0;
//size_t max_len = 0;
  char **lines = NULL,
       **plines,
        *buf   = NULL;

  if ( optind == argc  ) buf = (char *) RMloadstdin( &f_sz );
  else GTFOUT( "Only expecting stdin");

  if ( B_strip || B_onlys ) cleanbuf( buf );
  if ( B_onlys ){
    printf( "%s", buf );
    exit( 0 );
  }

  int *lens = line_lengths2( buf );

//max_len  = maxprintlen( buf );
  l_cnt = RMstr2arr( buf, "\n", &lines, -1 ) -1;

  if ( B_big ) B_colmn = l_cnt > hght;

  if ( l_cnt <= mrow ) B_colmn = false;   // disable if too few

  // 2020-12-07: adding '1' as buffer for double wide glyphs
  for ( int i=0; i<l_cnt; ++i ) max_len = MAX(max_len, lens[i] );
  max_len += wbuf;      // add spacing between columns
  min_len = max_len;
  for ( int i=0; i<l_cnt; ++i ) min_len = MIN(min_len, lens[i] > 1 ? lens[i]: min_len );
  for ( int i=0; i<l_cnt; ++i ) avg_len += lens[i];
//for ( int i=0; i<l_cnt; ++i ) STDOUT("%4d", lens[i] ); STDOUT("\n");
  avg_len /= l_cnt;
  min_len += min_len == 1;

  if ( debug )
    BUGOUT( "%d:min_len %d:max_len  %d:av_len %d:width %d:l_cnt\n",
        min_len,
        max_len, avg_len, wdth, l_cnt );

  int col_sz[16] = { 0 },
      cols = B_colmn ? wdth  / (max_len - ((max_len/min_len/2) -1 )) : 1;
  cols = cols > 0 ? cols : 1;
  int rows = l_cnt / cols    + ( (l_cnt%cols) ? 1 : 0);

  if ( debug )
    BUGOUT( "%d:cols  %d:rows\n", cols, rows );

//plines = lines;                      // only used if BUGOUT is reenabled
  for ( int i=0, k=0; i<cols; ++i ) {
    for ( int j=0; j<rows && k<l_cnt; ++j, ++k ) {
      col_sz[i] = MAX( col_sz[i], lens[k]+wbuf );    // 2021-01-10: adds wbuf space between columns
//    BUGOUT("%3d: %3d %3d :: %s\n", k, lens[i], col_sz[i], *(plines+k) );
    }
//  BUGOUT( "%d: %3d\t", i, col_sz[i] );
    if ( i>0 ) col_sz[i] += col_sz[i-1];
//  STDOUT( "%3d\n", col_sz[i] )
  }

//exit(0);

  plines = lines;
  int x;
  for ( int j=0, k=0; j<rows; ++j ) {
    for ( int i=0; i<cols; ++i, ++k ) {
      x = j + ( i * rows );

      // \r can be replaced with ^\e[0E which avoids getting
      // ^M with 'less -R' but it has weird artifacts
      // just use 'less -r' the next time you're trying to fix this
      printf( "%s\r\e[%dC",
         x<l_cnt ? *(plines+x) : "",
         (int) col_sz[i] );

    }
    printf( "\n" );
  }

#ifdef  ORIG_CODE
  l_cnt--;
  int mid = (l_cnt-0)/2 + (l_cnt%2);

  max_len = 0;
  for ( int i=0; i<mid; ++i ) max_len = MAX( (int) max_len, lens[i] );

  char **tlines =  lines + mid;
  plines = lines;
  for ( int j=0; j < mid; ++j, plines++, tlines++) {
    printf("%s\r[%dC %s\n",
       *plines, (int) max_len,
       *tlines ? *tlines : "xxx");
  }
#endif

  if ( debug & 0x0200 ) RMmalloc_debug( false  );

  exit(0);
}
