/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Prints tag data from files in a nice S-Expression

*/

#ifndef __ZF_PRINTID3
#define __ZF_PRINTID3

#include "id3v1.h"

void id3_unparse_v1(struct ID3v1 tag);
void id3_unparse_v2(void *src);

int id3_unparse(const char *fname);

#endif

