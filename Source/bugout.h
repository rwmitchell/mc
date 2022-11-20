#ifndef BUGOUT_H
#define BUGOUT_H

#include <libgen.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>

#define _FLC const char *F, int L, const char *C
#ifdef __APPLE__
#define _BLC basename_r( __FILE__, __dbuf ), __LINE__, __func__
#else
#define _BLC             __FILE__,           __LINE__, __func__
#endif

extern
char __dbuf[MAXNAMLEN];           // make name hidden/unique
char
     *RMtimer();
#ifdef  __APPLE__
#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
/* GTF routines change to use raise() instead of kill( 0, SIG )
 * to keep other instances of the same program from getting
 * the signal, avoid needing _itwasme
 */
#define GTFOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fprintf(stdout, " -- Exiting\n" ); \
  fflush (stdout); \
  raise( SIGCONT ); \
  exit( __LINE__ ); \
}
#define GTFERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fprintf(stderr, " -- Exiting\n" ); \
  fflush (stderr); \
  raise( SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", RMtimer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", basename_r(__FILE__, __dbuf), __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FLCOUT( F, L, C, FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s: ", basename_r( F, __dbuf), L, C ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FLCERR( F, L, C, FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s: ", basename_r( F, __dbuf), L, C ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#else
#define BUGOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define BUGERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#define GTFOUT( FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fprintf(stdout, " -- Exiting\n" ); \
  fflush (stdout); \
  raise( SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define GTFERR( FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fprintf(stderr, " -- Exiting\n" ); \
  fflush (stderr); \
  raise( SIGUSR1 ); \
  exit( __LINE__ ); \
}
#define TIMOUT( FMT, ... ) { \
  fprintf(stdout, "%s: ", RMtimer() ); \
  fprintf(stdout, "%s: %5d:%-24s:", __FILE__, __LINE__, __func__ ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FLCOUT( F, L, C, FMT, ... ) { \
  fprintf(stdout, ":%s: %5d:%-24s: ", F, L, C ); \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
  fflush (stdout); \
}
#define FLCERR( F, L, C, FMT, ... ) { \
  fprintf(stderr, ":%s: %5d:%-24s: ", F, L, C ); \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
  fflush (stderr); \
}
#endif
#define BUGNUL( FMT, ... ) { \
}                // Do nothing
#define STDOUT( FMT, ... ) { \
  fprintf(stdout, FMT, ##__VA_ARGS__ ); \
}
//fflush (stdout); \  // Removed STDOUT, big performance hit!
#define STDERR( FMT, ... ) { \
  fprintf(stderr, FMT, ##__VA_ARGS__ ); \
}
#define STDNUL( FMT, ... ) { \
}                // Do nothing

#endif  // BUGOUT_H
