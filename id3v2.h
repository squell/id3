/*

  No-bullshit ID3v2 support

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage (brief?):

  - ID3_readf() tries to read an ID3v2 tag from the file pointed to by
    fname into memory, and (if tagsize != NULL) writes the number of bytes
    read (excluding padding) in the integer pointed to by tagsize. It
    returns NULL on failure, a pointer to a region of memory holding
    the entire ID3v2 tag otherwise.

  - ID3_free() is used to clean up the pointer return by ID3_readf()

  - ID3_start() initializes f to be a iterator-like structure of the
    ID3v2 tag loaded into memory pointed to by buf. Call ID3_frame()
    afterwards to retrieve the first frame.

  - ID3_frame() moves the iterator & reports on the status (returns 0
    if there were no more frames to be read, 1 if there was.)

  - ID3_writef() writes the ID3v2 data pointed to by buf to the file
    pointed to by fname. Returns 1 on success, 0 on failure. The memory
    data pointed to by buf needs to have a terminating zero.

  - ID3_put() writes a frame of type ID to the memory location pointed to
    by dest, with contents pointed to by src, of length len, and returns
    an updated pointer pointing to a memory location where to write the
    next frame (if any). It takes care of terminating zeroes.

    If ID is invalid or src is NULL, ID3_put is equivalent to "*dest = 0".

  Example (reading a tag):

    void *buf = ID3_readf("filename",0);
    ID3FRAME f;

    if(buf) {
        ID3_start(frame, buf);
        while( ID3_frame(f) )
            ... do stuff with f->XXXX ...
    }

  Example (writing a tag):

    void *buf, *out;
    int i;

    out = ID3_put(buf, 0, 0, 0);

    for(i = 0; i < NUM; i++)
        out = ID3_put(out, frameid[i], data[i], size[i]);

    ID3_writef("filename", buf);

  Example (copying a tag):

    void *buf;
    if( (buf = ID3_readf("filename",0)) ) {
        ID3_writef("filename", buf);
    }

*/
#ifndef __ZF_ID3V2_H
#define __ZF_ID3V2_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {                           /* whole heaps of errors otherwise */
#endif

typedef struct _ID3FRAME {
    char *data;
    unsigned long size;
    char ID[5];
    int tag_volit  : 1;
    int file_volit : 1;
    int readonly   : 1;
    int packed     : 1;
    int encrypted  : 1;
    int grouped    : 1;
} ID3FRAME[1];

extern void   *ID3_readf(const char *fname, unsigned long *tagsize);
extern int     ID3_writef(const char *fname, void *buf);
extern void    ID3_free(void *buf);

extern void    ID3_start(ID3FRAME f, void *buf);
extern int     ID3_frame(ID3FRAME f);

extern void   *ID3_put(void *dest, const char ID[4], const void *src, size_t len);

#ifdef __cplusplus
}
#endif
#endif

