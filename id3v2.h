#ifndef __ZF_ID3V2_H
#define __ZF_ID3V2_H

#ifdef __cplusplus
extern "C" {                           /* whole heaps of errors otherwise */
#endif

typedef struct {
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

extern void   *ID3_put(void *dest, const char ID[4], void *src, size_t len);

#ifdef __cplusplus
}
#endif

#endif

