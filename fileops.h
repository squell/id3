/*

  Miscellaneous file operations

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

	  int cpfile(const char *srcnam, const char *dstnam);

  copies src to dest, using fcopy.

	  int fcopy(FILE *dest, FILE *src);

  copies (remainder of) file src to dest (both must be opened/usable)
  returns success or failure

	  int fpadd(FILE *dest, char c, size_t len);

  writes len times the character c to the file opened as dest.
  returns success or failure

	  FILE *fopentmp(const char *hint, char **name);

  tries to open a temporary file "near" the filename 'hint'. returns the
  file opened on success, NULL on failure. The filename generated is
  stored in a freshly allocated buffer which is assigned to *name.

*/

#ifndef __ZF_FILEOPS_H
#define __ZF_FILEOPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern int	 fcopy(FILE *dest, FILE *src);
extern int	 fpadd(FILE *dest, char c, size_t len);

extern FILE *fopentmp(const char *hint, char **name);

extern int	 cpfile(const char *srcnam, const char *dstnam);

#ifdef __cplusplus
}
#endif
#endif

