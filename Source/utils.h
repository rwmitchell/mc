#ifndef UTILS_H
#define UTILS_H

#ident "$Id$"

#include <stdio.h>     // off_t
#include "types.h"

/* Source/utils.c */
const char *RMbyte_to_binary(int x);
const char *RMUI32_to_binary(unsigned int x);
char *RMoff_str( off_t var );       // convert off_t to a string

#endif     // UTILS_H
