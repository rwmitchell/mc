#ifndef TYPES_H
#define TYPES_H

// $Id$

#define BIT_DEFS
#ifdef  BIT_DEFS
#  ifdef  USE_DEFINES
#  define SI08 char
#  define UI08 unsigned SI08
#  define SI16 short
#  define UI16 unsigned SI16
#  define SI32 int
#  define UI32 unsigned SI32
#  define SI64 long long
#  define UI64 unsigned SI64

#  define FP32 float
#  define FP64 double
#  else
  typedef float     FP32;
  typedef double    FP64;

  typedef char      byte;
  typedef char      SI08;
  typedef short     SI16;
  typedef int       SI32;
  typedef long      SI64;
  typedef long long S128;

  typedef unsigned char      UI08;
  typedef unsigned short     UI16;
  typedef unsigned int       UI32;
  typedef unsigned long      UI64;
  typedef unsigned long long U128;
# endif

#else // BYTE_DEFS          // 2020-06-20 will this ever be used?
#define SI1  char
#define UI1  unsigned SI1
#define SI2  short
#define UI2  unsigned SI2
#define SI4  int
#define UI4  unsigned SI4
#define SI8  long long
#define UI8  unsigned SI8
#endif

typedef unsigned char uchar;

typedef enum { False=0, True=1, Both=3, } Bool;

typedef union {   // Bool2
  struct {
    unsigned int enabled:1;
    unsigned int filled :1;
  };
  unsigned char X;
} Bool2;

#endif     // TYPES_H
