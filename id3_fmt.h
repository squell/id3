/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Present ID3 data in a readable fashion to the user

*/

#ifndef __ZF_ID3PRINT
#define __ZF_ID3PRINT

#include "id3v1.h"

struct id3_print {
     virtual ~id3_print()                    { }

     int operator()(const char *fname);

     virtual void file_beg(const char *)     { }
     virtual void file_end(void)             { }

     virtual void unparse_v1(struct ID3v1 &) { }
     virtual void unparse_v2(void *)         { }
};

#endif

