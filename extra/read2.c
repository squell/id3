#include <stdio.h>
#include "id3v2.h"

 /* id3v2.c interface demo */

int main(int argc, char *argv[])
{
    void *p = ID3_readf(argv[1], 0);
    ID3FRAME f;

    ID3_start(f,p);
    while( ID3_frame(f) )
        printf("%s\n\t%.*s\n", f->ID, (int)f->size-1, f->data+1);

    ID3_free(p);
}

