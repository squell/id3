/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Present ID3 data in a readable fashion to the user

*/

#ifndef __ZF_PRINTID3
#define __ZF_PRINTID3

#include "id3v1.h"

#ifdef __cplusplus
extern "C" {
#endif

void id3p_unparse_v1(struct ID3v1 tag);
void id3p_unparse_v2(void *src);

int id3p_showfile(const char *fname);

void id3p_listhead(void);
void id3p_listfoot(void);

#ifdef __cplusplus
}
#endif
#endif

