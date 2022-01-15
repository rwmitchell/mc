#ifndef LOADFILE_H
#define LOADFILE_H

#include <unistd.h>     // off_t
#include <stdbool.h>

/* Source/loadfile.c */
int RMfs_cnt(char *buf, char fs);
int RMcount_lines(char *fname);
int RMmax_linelen(char **arr);
int RMstr2arr(char *mlstr, const char *FS, char ***arr, int lim);
unsigned char *RMloadfile(const char *fname, off_t *f_sz, bool oride);
unsigned char *RMloadstdin(off_t *f_sz);
unsigned char *RMloadpipe( const char *cmd, off_t *f_sz );

#endif
