/*

  No-bullshit ID3v2 support

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage (brief?):

      void *ID3_readf(const char *fname, size_t *tagsize)

  reads ID3v2 tag from fname to memory, storing the tag size (w/o padding) in
  *tagsize (if != NULL). returns pointer on success, NULL on failure
  stores non-zero value in *tagsize if dangerous ID3v2 tag caused failure

      int ID3_writef(const char *fname, void *buf, size_t reqsize);

  writes in-memory tag pointed to by buf to fname. returns true / false on
  success / failure. in-memory tag needs to be zero-terminated.
  if reqsize != 0, tries to write id3v2 tag of that size
  [doesnt check consistency of fname (use ID3_readf first)!]

      void ID3_free(void *buf)

  frees pointer obtained by ID3_readf()

      void ID3_start(ID3FRAME f, void *buf)

  initializes frame-iterator for stepping through tag pointed to by buf

      int ID3_frame(ID3FRAME f)

  steps through a tag using iterator f. returns 0 if no more frames.

      void *ID3_put(void *dest, const char ID[4], const void *src, size_t len)

  writes a frame of type ID with contents *src (with size len) to the
  in-memory tag location pointed to by dest. returns the next memory
  location to write the next frame to. automatically adds zero-bytes.
  if ID is invalid or src == NULL, performs *(char*)dest = 0

      int (*ID3_wfail)(const char *dest, const char *src)

  this function pointer gets called if the rename() in ID3_writef fails.
  by default, this points to cpfile(). If this function returns 0, ID3_writef
  will abort the program with an error.

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

    out = ID3_put(buf, 0, 0, 0);
    for( .. frames to write .. )
        out = ID3_put(out, ..id, ..&data, ..sizeof data);

    ID3_writef("filename", buf);

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
    char *data;                           /* pointer to contents of frame */
    unsigned long size;                   /* length of frame contents     */
    char ID[5];                           /* ID of frame                  */
    int tag_volit  : 1;
    int file_volit : 1;
    int readonly   : 1;
    int packed     : 1;
    int encrypted  : 1;
    int grouped    : 1;
} ID3FRAME[1];

extern void   *ID3_readf(const char *fname, size_t *tagsize);
extern int     ID3_writef(const char *fname, void *buf, size_t reqsize);
extern void    ID3_free(void *buf);

extern void    ID3_start(ID3FRAME f, void *buf);
extern int     ID3_frame(ID3FRAME f);

extern void   *ID3_put(void *dest, const char ID[4], const void *src, size_t len);

extern int   (*ID3_wfail)(const char *srcname, const char *dstname);

#ifdef __cplusplus
}
#endif
#endif

