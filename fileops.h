/*

  Miscellaneous file operations

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

      char *tmpnam_alloc(const char *hint)

  generates a temporary filename "near" the filename 'hint', and stores
  this in freshly allocated memory. returns NULL on failure.

      FILE *opentemp(const char *hint, char **name)

  like tmpnam_alloc, but tries to open the file at the same time, storing
  the filename in *name if name != NULL

      int cpfile(const char *srcnam, const char *dstnam)

  copies src to dest, using fcopy.

      int fcopy(FILE *dest, FILE *src)

  copies (remainder of) file src to dest (both must be opened/usable)
  returns success or failure

      int fpadd(FILE *dest, char c, size_t len)

  writes len times the character c to the file opened as dest.
  returns success or failure

*/

#ifndef __ZF_FILEOPS_H
#define __ZF_FILEOPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern int   fcopy(FILE *dest, FILE *src);
extern int   fpadd(FILE *dest, char c, size_t len);

char        *tmpnam_alloc(const char *hint);
FILE        *opentemp(const char *hint, char **name);

extern int   cpfile(const char *srcnam, const char *dstnam);

#ifdef __cplusplus
}
#endif
#endif

