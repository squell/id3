#include "id3_scm.h"

 // use with id3v2_d.c

extern "C" int ID3_printf(char* fname, void *src)
{
    id3_scheme().unparse_v2(src);
}

