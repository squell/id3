/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Writes ID3 tag data as Scheme expressions.

*/

#ifndef __ZF_ID3PRINT_SCHEME
#define __ZF_ID3PRINT_SCHEME

#include "id3_fmt.h"
#include <cstdio>

#include "id3v1.h"
#include "id3v2.h"

struct id3_scheme : id3_print {
      id3_scheme()                    { std::printf("(define directory '("); }
     ~id3_scheme()                    { std::printf("))\n"); }

     void file_beg(const char *fname) { std::printf("\n(\"%s\"", fname); }
     void file_end(void)              { std::printf(")"); }

     void unparse_v1(struct ID3v1 &tag);
     void unparse_v2(void *src);
};

 // this should never actualy get inlined in production code, since they
 // should be called from within an instance-pointer with id3_print type

 // note that inline functions have external linkage so in any case you will
 // end up with just one copy of every function.

inline void id3_scheme::unparse_v1(struct ID3v1 &tag)
{
    using std::printf;
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

inline void id3_scheme::unparse_v2(void *src)
{
    using std::printf;
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

#endif

