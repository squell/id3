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

void id3p_unparse_v1(struct ID3v1 tag)
{
    printf("\n    (id3 (v 1.1)\n"
           "\t(title \"%.30s\")\n"
           "\t(artist \"%.30s\")\n"
           "\t(album \"%.30s\")\n"
           "\t(year \"%.4s\")\n"
           "\t(comment \"%.28s\")\n"
           "\t(track %d)\n"
           "\t(genre %d \"%s\"))",
           tag.title,
           tag.artist,
           tag.album,
           tag.year,
           tag.cmnt,
           tag.track,
           tag.genre,
           tag.genre < ID3v1_numgenres ?
             ID3v1_genre[tag.genre] : ""
          );
}

void id3p_unparse_v2(void *src)
{
    ID3FRAME frm;

    printf("\n    (id3v2 (v 3.0)");
    ID3_start(frm, src);
    while(ID3_frame(frm)) {
        int i, enc = frm->ID[0]       == 'T' ||
              strcmp(frm->ID, "WXXX") == 0   ||
              strcmp(frm->ID, "IPLS") == 0   ;
        char *p;

        printf("\n\t(%s \"", frm->ID);

        if(strcmp(frm->ID, "COMM") == 0) {
            printf("%.3s\" \"", frm->data+1);
            enc = 4;
        }

        for(i = frm->size-enc, p = frm->data+enc; i--; p++)
            if(enc && *p == '\0')
               printf("\" \"");
            else
               printf( (*p>=' ' && *p!='"')? "%c" : "#\\%c", *p );
        printf("\")");
    }
    printf(")");
}

int id3p_showfile(const char *fname)
{
    struct ID3v1 tag = { { 0 } };
    FILE *f   = fopen(fname, "rb");
    void *src;

    if(!f) return 0;

    printf("\n(\"%s\"", fname);

    if(f) {                                /* manually read ID3v1 tag */
        fseek(f, -128, SEEK_END);
        fread(&tag, 128, 1, f);
        fclose(f);
        if(memcmp(tag.TAG, "TAG", 3) == 0)
           id3p_unparse_v1(tag);
    }

    if( src=ID3_readf(fname, 0) ) {
        id3p_unparse_v2(src);
        ID3_free(src);
    }

    printf(")");
    return 1;
}

void id3p_listhead(void)
{
    printf("( ; Scheme S-Expression");
}

void id3p_listfoot(void)
{
    printf(")\n");
}


