#ident "$Id$"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // unlink()
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>      // open()
#include <errno.h>      // errno
// #include <sys/types.h>
#include <sys/stat.h>   // fstat()
#include "bugout.h"
#include "strings.h"    // truncname()
#include "io.h"
#include "utils.h"
#include "malloc.h"

const
char *RMbase  ( const char *nstr ) {
  const
  char *pstr1, *pstr2;

  pstr1 = nstr;
  pstr2 = strrchr( pstr1, '/');
  if ( !pstr2 ) pstr2 = nstr;
  else pstr2++;
  return(pstr2);
}
int   RMbehead_file( const char *fname, size_t head ) {
  char tmpname[] = "/tmp/.behead.XXXXXX";

  if ( mkstemp( tmpname ) >= 0 ) {    // 2021-02-11 creates file with 0600
    if ( RMskip_bytes( fname, tmpname, head ) < 0 ) {   // returns 0 on success
      BUGOUT( "RMskip_bytes(%s, %s) failed\n", fname, tmpname );
      return( -1 );
    }

    unlink( fname );
    RMmv( tmpname, fname );
  } else GTFOUT( "mkstemp(%s) failed", tmpname )

  return( 0 );
}
int   RMchange_ext ( const char *in, char *out, const char *old_ext, const char *new_ext ) {
  int ret = 0;
  char *pc;

  strcpy( out, in );

  if ( ( pc = strstr(out, old_ext)) == NULL ) {
    // fprintf(stderr, "did not find %s\n", old_ext );
    pc = out;
  } else {
    *pc = '\0';
  }
  strcat(pc, new_ext );

  return( ret );
}
int   RMcp    ( const char *in_name, const char *out_name, bool overwrite ) {
  int fd_in, fd_out,
      flags;

  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_in = open(in_name, O_RDONLY);
  if (fd_in < 0)
    return -1;

  flags = O_EXCL & !overwrite;
  fd_out = open(out_name, O_WRONLY | O_CREAT | flags, 0666);
  if (fd_out < 0)
    goto out_error;

  while (nread = read(fd_in, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;

    do {
      nwritten = write(fd_out, out_ptr, nread);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_out) < 0) {
      fd_out = -1;
      goto out_error;
    }
    close(fd_in);

    return 0;                          // Success!
  }

  out_error:
    perror(out_name);
    saved_errno = errno;

    close(fd_in);
    if (fd_out >= 0)
        close(fd_out);

    errno = saved_errno;
    return -1;

  return(0);
}
bool  RMexists( const char *path, const char *name ) {
  bool rv = false;
  char tmp[MAXNAMLEN];
  memset( tmp, '\0', MAXNAMLEN );
  if ( strlen( path ) > 0 ) {        // do not copy if path is blank
    strncat( tmp, path, MAXNAMLEN-2 );
    strncat( tmp, "/", 1);
  }
  strncat( tmp, name, MAXNAMLEN-(strlen(tmp) + strlen(name)) );

  if ( !access( tmp, R_OK ) ) rv = true;

#ifdef  DEBUG
  if ( debug & 0x0100 )
    BUGOUT("%d: %s\n", rv, tmp );
#endif

  return( rv );
}
off_t RMfdsize( int fd ) {           // return size of fd
  off_t size = -1L;
  struct stat buf;

  if ( fstat( fd, &buf ) < 0 )
    perror( __func__ );
  else
    size = buf.st_size;

  return( size );
}
int   RMfile_exists( const char *filename ) {
  struct stat buf;
  return( stat(filename, &buf) == 0 );
}
char *RMfindfile   ( const char *path, const char *name ) {
  bool found = false,
       error = false;
  static char tmp[MAXNAMLEN];

  strncpy(tmp, path, MAXNAMLEN);
  while ( !found && !error ) {
    found = RMexists( tmp, name );
    if ( ! found && strlen(tmp) > 0 ) {
      RMtruncname( tmp, tmp, '/' );
#ifdef  DEBUG
      if ( debug & 0x0100 )
        printf("%s:%d: path: %s\n", __func__, __LINE__, tmp );
#endif
    }
    error = strlen(tmp) <= 0 ;
  }
#ifdef __APPLE__
  strncat( tmp, "/", 1);
#else
  strcat( tmp, "/" );
#endif
  strncat( tmp, name, MAXNAMLEN-(strlen(tmp) + strlen(name)) );

  return( error ? (char *) NULL : tmp );
}
off_t RMfsize( const char *name ) {       // return size of the named file
  off_t size = -1L;
  struct stat sbuf;

  if (stat(name,&sbuf) == 0)
    if ( (sbuf.st_mode & S_IFMT) == S_IFREG)
      size=sbuf.st_size;

  return(size);
}
blksize_t RMget_blksize( int fd ) {
  struct stat buf;
  fstat( fd, &buf );
  return( buf.st_blksize );
}
bool  RMisdir     ( const char *name ) {
  struct stat sbuf;
  bool        isdir = false;

  if (stat(name,&sbuf) == 0)
    if ( S_ISDIR( sbuf.st_mode ) ) isdir = true;

  return( isdir );
}
bool  RMisfile    ( const char *name ) {
  struct stat sbuf;
  bool        isreg = false;

  if (stat(name,&sbuf) == 0)
    if ( S_ISREG( sbuf.st_mode ) ) isreg = true;

  return( isreg );
}
int   RMmkpath    ( const char *fullpath, mode_t mode ) {
  char oname[MAXNAMLEN],
       nname[MAXNAMLEN],
      *ptr, *ptr2;
  mode_t mask = 000, omask;
#ifdef arm
  struct group *grp;
#endif
  gid_t eaarl_gid = 700;
  // extern int errno;   // not all systems liked this.

  omask = umask(mask);  // when run as a daemon, owner is root/root

  if ( omask != mask )
    STDOUT( "RMmkpath: changed mask from %04o to %04o\n", omask, mask );

  strncpy(oname, fullpath, MAXNAMLEN);
  ptr2 = strrchr(oname, '/');
  *ptr2 = '\0';

  if ( mode == 0 ) mode = 0770;  // my default setting

  ptr = strtok(oname, "/");
  nname[0] = '\0';
#ifdef arm
  grp = getgrnam("eaarl");
  if ( grp != (struct group *) NULL ) eaarl_gid = grp->gr_gid;
  else fprintf(stdout, "RMmkpath: getgrnam failed, setting eaarl gid to %d\n", eaarl_gid);
#else
  // Something has broken this when compiling under vm1 ubuntu 9.04 and
  // running on ubuntu 10.04.  just hard code the value we want
  eaarl_gid = 700;
#endif

  while ( ptr != (char *) NULL) {
    strcat( nname, "/" );
    strncat( nname, ptr, (MAXNAMLEN-strlen(nname)) );
//  sprintf(nname, "%s/%s", nname, ptr);
    if ( access(nname, R_OK) == -1 ) {  // doesn't exist, must make
      mkdir(nname, mode);
    }
    // errno = 0;   // not all systems liked this.
    if ( chown(nname, -1, eaarl_gid) == -1 ) {
      perror( "RMmkpath: chown: " );
      STDOUT( "RMmkpath: chown: %s FAILED\n", nname);
    }        // set group to eaarl
    ptr = strtok((char *) NULL, "/");
  }

  return(0);
}
int   RMmv        ( const char *oname,  const char *nname ) {
  int status=0;
  if (access(oname,R_OK) != 0)  // input  file does not exist
    status = -1;
  else {
    if (access(nname,R_OK) == 0)  // output file already exists
      status = -2;
    else {
      if (link(oname,nname) != -1) unlink(oname);
      else status= -3;
    }
  }
  return(status);
}
char *RMnoext     ( char *nstr, bool B_all, bool B_period ) {
  char *pstr1, *pstr2;
  char ext = '.';

  pstr2 = strrchr( nstr, '/');
  if ( !pstr2 ) pstr2 = nstr;

  if ( B_all ) pstr1 = strchr (pstr2, ext);
  else         pstr1 = strrchr( nstr, ext);
  if ( pstr1 != NULL && pstr1 > pstr2 ) {
    if ( B_period && *pstr1 == ext ) *(pstr1+1) = '\0';
    else *pstr1 = '\0';
  }
  return(nstr);
}
int   RMrm        ( const char *name ) {
  int status=0;
  if (access(name,R_OK) == 0)  // file exists
    unlink(name);
  else status= -1;
  return(status);
}
int   RMskip_bytes( const char *name_in, const char *name_out, size_t skip ) {
  int fd_in,
      fd_out;
  int buf_sz;
  size_t rv, rvw;
  char *buffer = NULL;
  bool B_done = false;

  if ( (rv = RMfsize( name_in )) <= skip ) {
    BUGOUT( "%s is only %lu bytes, can't skip %lu bytes\n", name_in, rv, skip );
    return( -1 );
  }

  if ( ( fd_in=open( name_in, O_RDONLY )) < 0 ) {
    BUGOUT( "Failed to open %s for reading\n", name_in );
    perror( name_in );
    return( -1 );
  }

  if ( ( fd_out=open( name_out, O_WRONLY|O_CREAT|O_EXCL, 0644 )) < 0 ) {
    BUGOUT( "Failed to open %s for writing\n", name_out );
    perror( name_out );
    return( -2 );
  }

  if ( (rv = lseek( fd_in, skip, SEEK_SET )) != skip ) {
    BUGOUT( "Failed to skip %lu bytes, rv = %lu\n", skip, rv );
    perror ("lseek ");
    return( -3 );
  }

  // Allocate buffer based on optimal read buffer
  buf_sz = RMget_blksize( fd_in );
  buffer = RMmalloc( buffer, sizeof( char ) * buf_sz );
  if ( buffer == NULL ) {
    BUGOUT( "Falled to malloc\n");
    return( -4 );
  }
  while ( !B_done && (rv = read(fd_in, buffer, buf_sz )) > 0 ) {
    if ( ( rvw = write( fd_out, buffer, rv ) ) != rv ) {
      BUGOUT( "failed to write %lu bytes, wrote %lu\n", rv, rvw );
      B_done = true;
    }
  }
  RMfree( buffer );
  close( fd_in  );
  close (fd_out );

  return( 0 );
}
int   RMwildcard  ( char *name, const char *pat ) {
  static
  char curpat[ MAXNAMLEN   ];
  char    cmd[ MAXNAMLEN+5 ];   // space for 'echo '
  static
  FILE *pipe = NULL;
  int   ret = 0;

  name[0] = '\0';
  if ( strncmp( pat, curpat, MAXNAMLEN ) ) {      // patterns don't match, start a new search
    if ( pipe != NULL ) pclose( pipe );

    strncpy( curpat, pat, MAXNAMLEN );
    sprintf(cmd, "echo %s", curpat );

    pipe = popen(cmd, "r");
  }

  if ( pipe != NULL )
    ret = fscanf( pipe, "%s", name );

  if ( ! strncmp( name, curpat, MAXNAMLEN ) ) {   // got pattern back, not filename match
    name[0] = '\0';
    ret = -2;
  }

  if ( ret < 0 ) {
    pclose( pipe );
    pipe = NULL;
  }

  return(ret);
}
