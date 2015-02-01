/*

  No-bullshit ID3v2 support

  copyright (c) 2003-2006, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage (brief?):

      void *ID3_readf(const char *fname, size_t *tagsize)

  reads ID3v2 tag from fname to memory, storing the tag size (w/o padding) in
  *tagsize (if != NULL). returns pointer on success, NULL on failure
  stores zero in *tagsize on failure, or non-zero value if a malformed ID3v2
  tag caused failure.

      int ID3_writef(const char *fname, const void *buf, size_t reqsize);

  writes in-memory tag pointed to by buf to fname. returns true / false on
  success / failure. in-memory tag needs to be zero-terminated.
  if reqsize != 0, tries to write id3v2 tag of that size.
  if buf is NULL or points to an empty tag, deletes tag.
  [doesnt check consistency of fname (use ID3_readf first)!]

      void ID3_free(const void *buf)

  frees pointer obtained by ID3_readf()

      ID3VER ID3_start(ID3FRAME f, const void *buf)

  initializes frame-iterator for stepping through tag pointed to by buf

      int ID3_frame(ID3FRAME f)

  steps through a tag using iterator f. returns 0 if no more frames.

      void *ID3_put(void *dest, ID3VER version, const char ID[4], const void *src, size_t len)

  writes a frame of type ID with contents *src (size len) to the in-memory tag
  location pointed to by dest. returns the next memory location to write the
  next frame to. first call with ID null to perform initialization. version
  must be 2 or 3 (for ID3v2.2 or ID3v2.3). A mixed version tag will result in
  undefined behaviour by ID3_writef. returns dest to indicate error (no-op).

      void (*ID3_wfail)(const char *dest, const char *src)

  this function pointer gets called if the mvfile() in ID3_writef fails.
  by default, this points to an abort.

  Example (reading a tag):

    void *buf = ID3_readf("filename",0);
    ID3FRAME f;

    if(buf) {
        ID3_start(frame, buf);
        while( ID3_frame(f) )
            ... do stuff with f->XXXX ...
    }

  Example (writing a tag):

    char buf[10000], *out;
    int i;

    out = ID3_put(buf, ID3_v2_3, 0, 0, 0);
    for( .. frames to write .. )
        out = ID3_put(out, ID3_v2_3, ..id, ..&data, ..sizeof data);

    ID3_writef("filename", buf, 0);

  Notes:

  ID3_writef() is very conservative by default: it will only rewrite the
  MP3 file if it absolutely has to, i.e., if a tag has to be removed,
  added, or is too small. It will not automatically rewrite MP3 files
  to downsize ID3v2 tags (that would be silly).

*/
#ifndef __ZF_ID3V2_H
#define __ZF_ID3V2_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {                           /* whole heaps of errors otherwise */
#endif

typedef struct _ID3FRAME {
    const char *data;                     /* pointer to contents of frame */
    unsigned long size;                   /* length of frame contents     */
    char ID[5];                           /* ID of frame                  */
    unsigned tag_volit  : 1;
    unsigned file_volit : 1;
    unsigned readonly   : 1;
    unsigned packed     : 1;
    unsigned encrypted  : 1;
    unsigned grouped    : 1;
    unsigned _rev       : 2;
} ID3FRAME[1];

typedef enum {
    ID3_v2_2 = 2,
    ID3_v2_3 = 3
} ID3VER;

extern void   *ID3_readf(const char *fname, size_t *tagsize);
extern int     ID3_writef(const char *fname, const void *buf, size_t reqsize);
extern void    ID3_free(const void *buf);

extern ID3VER  ID3_start(ID3FRAME f, const void *buf);
extern int     ID3_frame(ID3FRAME f);

extern void   *ID3_put(void *dest, ID3VER version, const char ID[4], const void *src, size_t len);

extern void  (*ID3_wfail)(const char *srcname, const char *dstname);

#ifdef __cplusplus
}
#endif
#endif

