#ifndef STRINGS_H
#define STRINGS_H

/* Source/strings.c */
char *RMadjustl(char *str, int len);
char *RMrtrim(char *str, int len);
char *RMtruncname(char *in, char *out, char sep);
char *RMlower( char *str );
char *RMstrcasestr( const char *haystack, const char *needle );

#endif     // STRINGS_H
