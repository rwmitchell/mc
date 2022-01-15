#ident "$Id: mylib malloc.c default b4c53c1ce6ab 2021-12-27 00:01:33Z rwmitchell $"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "types.h"
#include "bugout.h"
#include "malloc.h"

// ****** malloc routines ******

static RMptr_t *RMptr     = NULL;
static UI32     RMptr_cnt = 0,
                RMptr_tot = 0;

/* smarter malloc()
 * use a separate array to track size allocated
 * RMptr[] initially created with 32 elements
 * gets realloced when _cnt == _tot
 * using the pointer address, RMmalloc_size() looks
 * thru the array to find the same pointer, then
 * returns the associated len field.
 */

void   _RMmalloc_debug( _FLC, bool flag ) {
  RMptr_t *p = RMptr;
  UI64 tot=0,
       cnt=0;

  FLCOUT( F,L,C, "%d malloc table entries\n", RMptr_cnt )

  for ( UI32 id=0; id<RMptr_cnt; id++, p++ )
    if ( flag || p->ptr ) {
      tot += p->len;
      cnt++;

      STDOUT( "RMmalloc_debug:%3d: %10lu %14p %4d %8s %s %s\n",
          id,
          p->len, p->ptr, p->line,
          p->nvar,
          p->file, p->func )
    }
  FLCOUT( F,L,C, "RMmalloc_debug %lu items: %lu bytes  %.3lf MB  %.4lf GB\n",
    cnt, tot, (FP64) tot/(1024*1024), (FP64) tot/(1024*1024*1024) )
}
UI32    RMmalloc_cnt  ( ) {
  RMptr_t *p = RMptr;
  UI32 cnt = 0;

  for ( UI32 i=0; i<RMptr_cnt; ++i, ++p ) if ( p->ptr ) ++cnt;
  return( cnt );
}
size_t  RMmalloc_len  ( ) {
  RMptr_t *p = RMptr;
  size_t len = 0;

  for ( UI32 i=0; i<RMptr_cnt; ++i, ++p ) if ( p->ptr ) len += p->len;
  return( len );
}
static
SI32    RMmalloc_id   ( void *pointer ) {
  SI32 id = -1;
  RMptr_t *p = RMptr;

  for ( UI32 i=0; i < RMptr_cnt && id < 0; i++, p++ ) {
    if ( p->ptr == pointer ) id = i;
  }

  return( id );
}
SI32   _RMfree        ( _FLC, void *pointer, const char *str ) {
  SI32   id = -1;

  if ( ! pointer) return ( id );

  if ( (id = RMmalloc_id( pointer )) >= 0 ) {
    RMptr_t *p = &RMptr[id];
//  FLCOUT( F,L,C, "FREE: %d %8lu %p\n", id, p->len, p->ptr )
    free( p->ptr );
    free( p->file);
    free( p->func);
    free( p->nvar);
    p->len = 0;
    pointer= NULL;     // this does nothing, pointer address not returned
    p->ptr = NULL;
    p->file= NULL;
    p->func= NULL;
    p->nvar= NULL;
    p->line = 0;
  } else
    FLCOUT( F,L,C, "%p '%s' not registered\n", pointer, str )
  return( id );
}
size_t _RMmalloc_size ( _FLC, void *pointer, const char *str ) {
  SI32   id = -1;
  size_t sz = -1;

  if ( (id = RMmalloc_id( pointer )) >= 0 )
    sz = RMptr[id].len;

  if ( pointer && id < 0 )
    FLCOUT( F,L,C, "%p '%s' not registered\n", pointer, str )

  return( sz );
}
void    RMptr_init    ( ) {
  static size_t sz_RMptr = sizeof( RMptr_t ),
                blk_sz = 0;

  if ( RMptr == NULL ) {                            // create initial RMptr array
    RMptr_tot = 32;
    blk_sz = sz_RMptr * RMptr_tot;
    RMptr = malloc( blk_sz );
    memset( RMptr, '\0', blk_sz );
  } else if ( RMptr_cnt >= RMptr_tot ) {            // increase RMptr array
    RMptr_tot += 32;

#ifdef __APPLE__
    RMptr = (RMptr_t *) reallocf( RMptr, sz_RMptr * RMptr_tot );
#else
    RMptr = (RMptr_t *) realloc ( RMptr, sz_RMptr * RMptr_tot );
#endif

    if ( !RMptr ) GTFOUT( "reallocf(%u) failed", RMptr_tot )

    void *p = RMptr + ( RMptr_tot - 32 );         // find start of new memory
    memset( p, '\0', blk_sz );
  }

}
void  *_RMrealloc     ( _FLC, void *pointer, size_t size, const char *str ) {
  SI32 id = -1;
  RMptr_t *p;

//FLCOUT( F,L,C, "Reallocate %lu for %s\n", size, str )

  RMptr_init();

  if ( (id = RMmalloc_id( pointer )) < 0 ) id = RMptr_cnt++;
  p = &RMptr[id];

#ifdef __APPLE__
  p->ptr = pointer = reallocf( p->ptr, size );
#else
  p->ptr = pointer = realloc ( p->ptr, size );
#endif

//FLCOUT( F, L, C, "CHECK %p: %lu -> %lu\n", p, p->len, size )

  if ( ! p->ptr ) {
    FLCOUT( F, L, C, "%p %s failed to allocate %lu bytes\n", pointer, str, size );
    exit( -1 );
  }

  if ( size > p->len )           // clear new allocation
    memset( (p->ptr+p->len), '\0', size - p->len );
//  memset( &p->ptr[p->len], '\0', size - p->len );

  if ( p->file ) free( p->file );
  if ( p->func ) free( p->func );
  if ( p->nvar ) free( p->nvar );

  p->len  = size;
  p->line = L;
  p->file = strdup( F );
  p->func = strdup( C );
  p->nvar = strdup( str );

  return( p->ptr );
}
void  *_RMmalloc      ( _FLC, void *pointer, size_t size, const char *str ) {
  SI32 id = -1;
  RMptr_t *p;    //  = RMptr;

//FLCOUT( F,L,C, "Allocate %lu for %s\n", size, str )

  RMptr_init();  // make sure RMptr is allocated

  if ( RMptr_cnt > 0 ) {                            // is table populated?
    id = _RMfree( F,L,C, pointer, str);
  }

  if ( id < 0 ) id = RMptr_cnt++;                   // Need a new pointer

  p = &RMptr[id];                                   // set 'p' to slot
  p->len = size;
  p->ptr = pointer =  malloc( p->len );
  p->line = L;
  p->file = strdup( F );
  p->func = strdup( C );
  p->nvar = strdup( str );

  if ( pointer == NULL ) {
    FLCOUT( F,L,C, "Failed to allocate %lu for %s bytes, exiting\n", size, str )  // go to any user redirection
    FLCERR( F,L,C, "Failed to allocate %lu for %s bytes, exiting\n", size, str )  // go to the screen
    exit(0);                // no need to check return value for NULL, we've already exited
  }
  memset( pointer, '\0', size );
  return( pointer );
}
char  *_RMstrdup      ( _FLC, const char *s1, const char *str ) {
  char *s2 = strdup( s1 );
  SI32 id = RMmalloc_id( NULL );
  RMptr_t *p;

//FLCOUT( F,L,C, "Dupe %s\n", str )

  RMptr_init();  // make sure RMptr is allocated

  if ( id < 0 ) id = RMptr_cnt++;                   // Need a new pointer
  p = &RMptr[id];

  p->len = strlen( s2 );
  p->ptr = s2;
  p->line = L;
  p->file = strdup( F );
  p->func = strdup( C );
  p->nvar = strdup( str );

  if ( ! s2 )
    FLCOUT( F,L,C, "%3d %s <%s>: %p %8ld\n",  id, s1, str, s2, RMptr[id].len )

  return( s2 );
}

#ifdef DEPRECATED
void *RM_malloc( _FLC, void *pointer, size_t size ) {    // USE RMmalloc()
  // DO NOT USE this routine - use RMmalloc(), which then calls this
  if ( debug & 0xF00000) FLCOUT( F, L, C, "\n" )
  if ( pointer != NULL ) free( pointer );

  pointer =  malloc( size );
  if ( pointer == NULL ) {
    FLCOUT( F, L, C, "Failed to allocate %lu bytes, exiting\n", size )  // go to any user redirection
    FLCERR( F, L, C, "Failed to allocate %lu bytes, exiting\n", size )  // go to the screen
    exit(0);                // no need to check return value for NULL, we've already exited
  }
  memset( pointer, '\0', size );
  return( pointer );
}
#endif
