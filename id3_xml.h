/*

  ID3 printing function

  (c) 2003 squell ^ zero functionality!
  ** DO NOT DISTRIBUTE - FOR PRIVATE USE ONLY **

  Writes ID3 tag data as (urrgghh!!) XML "data"

*/

#ifndef __ZF_ID3GARBAGE_XML
#define __ZF_ID3GARBAGE_XML

#include "id3_fmt.h"
#include <cstdio>
#include "id3v1.h"
#include "id3v2.h"

struct id3_xml : id3_print {
      id3_xml();
     ~id3_xml();

     void file_beg(const char *fname);
     void file_end(void);

     void unparse_v1(struct ID3v1 &tag);
     void unparse_v2(void *src);
};

inline id3_xml::id3_xml()
{   std::printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n");
    std::printf("<Directory>\n");   }

inline id3_xml::~id3_xml()
{   std::printf("</Directory>\n"); }

inline void id3_xml::file_beg(const char *fname)
{   std::printf("  <File name=\"%s\">\n", fname); }

inline void id3_xml::file_end(void)
{   std::printf("  </File>\n"); }

inline void id3_xml::unparse_v1(struct ID3v1 &tag)
{
    std::
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
           tag.genre,
           tag.genre < ID3v1_numgenres ?
             ID3v1_genre[tag.genre] : ""
          );
}

inline void id3_xml::unparse_v2(void *src)
{
    using std::printf;
    ID3FRAME frm;

    printf("    <Tag type=\"ID3\" ver=\"2.3\">\n");
    ID3_start(frm, src);

    while(ID3_frame(frm)) {
        int i, enc = frm->ID[0]       == 'T' ||
              strcmp(frm->ID, "WXXX") == 0   ||
              strcmp(frm->ID, "IPLS") == 0   ;
        char *p;

        printf("      <%s",  frm->ID);
        if(strcmp(frm->ID, "COMM") == 0) {
            printf(" lang=\"%.3s\"", frm->data+1);
            enc = 4;
        }
        printf(">");

        for(i = frm->size-enc, p = frm->data+enc; i--; p++)
            switch(*p) {
            case 0   : printf("<NULL/>"); break;
            case '<' : printf("&lt;");   break;
            case '>' : printf("&gt;");   break;
            case '\'': printf("&apos;"); break;
            case '\"': printf("&quot;"); break;
            case '&' : printf("&amp;");   break;
            default:
                printf( (*p>=' ')? "%c" : "&#%d;", *p );
            }
        printf("</%s>\n", frm->ID);
    }
    printf("    </Tag>\n");
}

#endif

