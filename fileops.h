/*

  Miscellaneous file operations

  (c) 2003, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

      FILE *opentemp(const char *hint, char **name)

  generates a temporary file "near" the filename 'hint' using ftemp, stores
  the name (if != NULL), and returns the open stream.

      int cpfile(const char *srcnam, const char *dstnam)

  copies src to dest, using fcopy.

      int fcopy(FILE *dest, FILE *src)

  copies (remainder of) file src to dest (both must be opened/usable)
  returns success or failure

      size_t fpadd(char c, size_t len, FILE *dest)

  writes len times the character c to the file opened as dest.
  returns number of character actually written

      FILE *ftemp(char *templ, const char *mode)

  behaves like mk(s)temp, but returns a stream opened in mode

*/

#ifndef __ZF_FILEOPS_H
#define __ZF_FILEOPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern int    fcopy(FILE *dest, FILE *src);
extern size_t fpadd(char c, size_t len, FILE *dest);
extern FILE  *ftemp(char *templ, const char *mode);

extern FILE  *opentemp(const char *hint, char **name);

extern int    cpfile(const char *srcnam, const char *dstnam);

#ifdef __cplusplus
}
#endif
#endif

