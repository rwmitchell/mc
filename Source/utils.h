#ifndef UTILS_H
#define UTILS_H

#ident "$Id: mylib utils.h default 6fb61d66b65c 2021-02-11 17:41:34Z rwmitchell $"

#include <stdio.h>     // off_t
#include "types.h"

/* Source/utils.c */
const char *RMbyte_to_binary(int x);
const char *RMUI32_to_binary(unsigned int x);
char *RMoff_str( off_t var );       // convert off_t to a string

#endif     // UTILS_H
