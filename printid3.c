/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Prints (in-memory) tag data

*/

#ifndef __ZF_PRINTID3
#define __ZF_PRINTID3

#include <stdio.h>
#include <string.h>
#include "id3v1.h"
#include "id3v2.h"

#define countof(x) (sizeof x/sizeof*x)

void id3_unparse_v1(struct ID3v1 tag)
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
           tag.genre < countof(ID3v1_genres) ?
             ID3v1_genres[tag.genre] : ""
          );
}

void id3_unparse_v2(void *src)
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

int id3_unparse(const char *fname)
{
    struct ID3v1 tag = { { 0 } };
    void *src = ID3_readf(fname, 0);
    FILE *f   = fopen(fname, "rb");

    printf("\n(\"%s\"", fname);

    if(f) {                                /* manually read ID3v1 tag */
        fseek(f, -128, SEEK_END);
        fread(&tag, 128, 1, f);
        fclose(f);
        if(memcmp(tag.TAG, "TAG", 3) == 0)
           viewid_unparse_v1(tag);
    }

    if(src)
        viewid_unparse_v2(src);

    printf(")");
}

 /* to be removed */

int ID3_xml(const char *fname)
{
    struct ID3v1 tag = { { 0 } };
    FILE *f;

    ID3FRAME frm;
    char *p, *src = ID3_readf(fname,0);
    int i;

    printf("  <File name=\"%s\">\n", fname);

    if(f = fopen(fname, "rb+")) {        /* manually read ID3v1 tag */
        fseek(f, -128, SEEK_END);
        fread(&tag, 128, 1, f);
        fclose(f);
        if(memcmp(tag.TAG, "TAG", 3) == 0) {
            printf("    <Tag type=\"ID3\" ver=\"1.1\">\n"
                   "      <title>%.30s</title>\n"
                   "      <artist>%.30s</artist>\n"
                   "      <album>%.30s</album>\n"
                   "      <year>%.4s</year>\n"
                   "      <comment>%.28s</comment>\n"
                   "      <track>%d</track>\n"
                   "      <genre id=\"%d\">%s</genre>\n"
                   "    </Tag>\n",
                   tag.title,
                   tag.artist,
                   tag.album,
                   tag.year,
                   tag.cmnt,
                   tag.track,
                   tag.genre, ID3v1_genres[tag.genre]);
        }
    }

    if(src) {
        printf("    <Tag type=\"ID3\" ver=\"2.3\">\n");
        ID3_start(frm, src);
        while(ID3_frame(frm)) {
            printf("      <%s><text>",  frm->ID);
            for(i = frm->size, p = frm->data; i--; p++)
                switch(*p) {
                case 0   : printf("</text><text>"); break;
                case '<' : printf("&lt;");   break;
                case '>' : printf("&gt;");   break;
                case '\'': printf("&apos;"); break;
                case '\"': printf("&quot;"); break;
                case '&': printf("&amp;");   break;
                default:
                    printf( *p>=' ' ? "%c" : "&#%d;", *p );
                }
            printf("</text></%s>\n", frm->ID);
        }
        printf("    </Tag>\n");
    }
    printf("  </File>\n");
}

int _main(int argc, char* argv[])
{
    printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n");
    printf("<?xml-stylesheet type=\"text/xsl\" href=\"layout.xsl\" ?>\n\n");

    printf("<Directory files=\"%d\">\n", argc-1);
    for( ; --argc; ID3_xml(*++argv));
    printf("</Directory>\n");
}

int main(int argc, char* argv[])
{
    printf("(define directory '(");

    for( ; --argc; viewid_file(*++argv));

    printf("))\n");
}

#endif
