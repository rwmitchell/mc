#ident "$Id$"

#include <stdio.h>
#include <stdlib.h>    // strtol()
#include <pthread.h>   // timespec
#include <sys/time.h>  // NOW()
#include <math.h>      // ceill()
#include <string.h>
#define BIT_DEFS
#include "types.h"
#include "bugout.h"
#include "utils.h"

extern int debug;

const char *RMbyte_to_binary( int x ) {
  static char b[9];
  int z;

  memset(b, '\0', 9);
  for (z=128; z>0; z >>= 1)
    strcat(b, ((x & z) == z) ? "1" : "0");

  return b;
}


const char *RMUI32_to_binary( UI32 x ) {
  static char b[48];
  UI64 z;

  // printf("byte: %0x\n", x );
  memset(b, '\0', 48);
  for ( z=0x080000000ULL; z>0; z >>= 1) {  // show 32 bits
    if ( z & 0x008888888ULL ) strcat( b, " ");
    strcat(b, ((x & z) == z) ? "1" : "0");
  }
  // printf("\n");

  return b;
}
const char *RMUI64_to_binary( UI64 x ) {
  static char b[64];
  UI64 z;

  // printf("byte: %0x\n", x );
  memset(b, '\0', 64);
  for ( z=   0x080000000000000ULL; z>0; z >>= 1) {  // show more bits
    if ( z & 0x008888888888888ULL ) strcat( b, " ");
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}
const char *RMUI32_to_binary2( UI32 x, UI32 map ) {
  static char b[64];
  UI64 z;

  // printf("byte: %0x map: %0X\n", x, map );
  memset(b, '\0', 64);
  for ( z=0x080000000ULL; z>0; z >>= 1) {  // show 32 bits
    if ( z & map ) strcat( b, " ");
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

int RMUI32_to_ints( UI32 x, UI32 map, UI16 a[] ) {
  int  j = 0;
  UI64 z;

  a[j] = 0;

  for ( z=0x080000000ULL; z>0; z >>= 1) {  // show 32 bits
    if ( z & map ) a[++j] = 0;

    a[j] = ( a[j]<< 1 ) + ( ( (x & z) == z) ? 1 : 0);
  }

  return j;
}

void *RMascii2hex( char *in,  char *out, int len ) {
  int i;
  char *pout, tmp[3];

  tmp[2] = '\0';

  // printf("ASCII2HEX %d: %s", len, in ); fflush(stdout);
  pout = out;
  for ( i=0; i<len; i+=2 ) {
    tmp[0] = *in++;
    tmp[1] = *in++;
    *pout++ = strtol(tmp, NULL, 16);
  }

#ifdef DEBUG
  pout = out;
  for ( i=0; i<len/2; ++i )
    printf("%02x ", out[i]&0x00FF);
  printf("\n");
#endif

  return(NULL);    // 2017-01-04: not sure why compiler needs this

}
void RMhex2ascii( char *in, char *out, int len ) {
  int i,
      crc = 0;

  for (i=0; i<len; ++i ) {         // convert to asciified hex string.
    sprintf(out, "%02X ", in[i] & 0x00FF );
    crc ^= in[i];                  // compute CRC value
    out += 2;
  }
  sprintf(out, "%02X ", crc & 0x00FF );  // append CRC value
}
void RMdump_hex( char *label, char *ptmp, int len ) {
  int i, j;
  long maxi = ceill(len/16.0);

  printf("%s %ld: %s\n", __func__, maxi, label );
  for (i=0; i<maxi; ++i ) {
    for ( j=0; j<4; ++j )
      printf("%02x%02x %02x%02x ",
        (ptmp)[(i*16)+(j*4)+1]&0xFF,
        (ptmp)[(i*16)+(j*4)+0]&0xFF,
        (ptmp)[(i*16)+(j*4)+3]&0xFF,
        (ptmp)[(i*16)+(j*4)+2]&0xFF);
    printf("\n");fflush(stdout);
  }
  printf("\n");fflush(stdout);
}
unsigned short RMgetbits( short beg, short end, short mask, UI32 val) {
  end=0;   // just to quiet YCM
  // "end" is no longer used, but left to document routine
  // yes, the mask can be easily calculated, but costly being done so often.
  // mask = (int) (pow(2, end-beg+1)-1);

  if ( debug ) BUGOUT( "%hd  to %hd\n", beg, end );
  return (val << beg) & mask;
}
char *RMoff_str( off_t var ) {       // convert off_t to a string - don't use more than once in output
  static
  char str[13];
#ifdef __linux__
  sprintf( str, "%12ld", var);
#else
  sprintf( str, "%12lld", var);
#endif
  return( str  );
}
