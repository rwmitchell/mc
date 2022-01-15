#ifndef MALLOC_H
#define MALLOC_H

#ident "$Id: mylib malloc.h default 2423d79e4b35 2020-12-21 12:39:51Z rwmitchell $"

#include <stdlib.h>
#include "bugout.h"
#include "types.h"

typedef struct {       // RMptr_t
  size_t len;
  UI32   line;
  char  *file,
        *func,
        *nvar;         // var name
  void  *ptr;
} RMptr_t;

void   _RMmalloc_debug( _FLC, bool flag );
size_t _RMmalloc_size ( _FLC, void *pointer,              const char *str );
SI32   _RMfree        ( _FLC, void *pointer,              const char *str );
void  *_RMrealloc     ( _FLC, void *pointer, size_t size, const char *str );
void  *_RMmalloc      ( _FLC, void *pointer, size_t size, const char *str );
char  *_RMstrdup      ( _FLC, const char *s1,             const char *str );

#define RMmalloc_debug( flg ) _RMmalloc_debug( _BLC, flg )
#define RMmalloc_size(  ptr ) _RMmalloc_size( _BLC, ptr,     #ptr )
#define RMfree(    ptr      ) { _RMfree     ( _BLC, ptr,     #ptr ); ptr=NULL; }
#define RMrealloc( ptr, sz  ) _RMrealloc    ( _BLC, ptr, sz, #ptr )
#define RMmalloc(  ptr, sz  ) _RMmalloc     ( _BLC, ptr, sz, #ptr )
#define RMstrdup(  ptr      ) _RMstrdup     ( _BLC, ptr,     #ptr )

#endif
