/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Writes ID3 tag data as Scheme expressions.

*/

#include <stdio.h>
#include <string.h>
#include "id3v1.h"
#include "id3v2.h"

#include "id3_fmt.h"

id3_print& id3_print::operator()(const char *fname)
{
    struct ID3v1 tag = { { 0 } };
    FILE *f   = fopen(fname, "rb");
    void *src;

    file_beg(fname);

    if(f) {                                /* manually read ID3v1 tag */
        fseek(f, -128, SEEK_END);
        fread(&tag, 128, 1, f);
        fclose(f);
        if(memcmp(tag.TAG, "TAG", 3) == 0)
           unparse_v1(tag);
    }

    if( src=ID3_readf(fname, 0) ) {
        unparse_v2(src);
        ID3_free(src);
    }

    file_end();
    return *this;
}

