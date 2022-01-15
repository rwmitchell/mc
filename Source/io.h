#ifndef IO_H
#define IO_H

#ident "$Id: mylib io.h default 5c7ca527220b 2021-01-06 11:47:38Z rwmitchell $"

#include <unistd.h>
#include <sys/stat.h>

/* Source/io.c */
const
char *RMbase       (const char *nstr);
int   RMbehead_file(const char *fname, size_t head);
int   RMchange_ext (const char *in, char *out, const char *old_ext, const char *new_ext);
int   RMcp         (const char *in_name, const char *out_name, _Bool overwrite);
_Bool RMexists     (const char *path, const char *name);
off_t RMfdsize     (int fd);
int   RMfile_exists(const char *filename);
char *RMfindfile   (const char *path, const char *name);
off_t RMfsize      (const char *name);
blksize_t RMget_blksize(int fd);
_Bool RMisdir      (const char *name);
_Bool RMisfile     (const char *name);
int   RMmkpath     (const char *fullpath, mode_t mode);
int   RMmv         (const char *oname, const char *nname);
char *RMnoext      (char *nstr, _Bool B_all, _Bool B_period);
int   RMrm         (const char *name);
int   RMskip_bytes (const char *name_in, const char *name_out, size_t skip);
int   RMwildcard   (char *name, const char *pat);

#endif     // IO_H
