#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "fileops.h"
#include "id3v2.h"

/*

  (c) 2003-2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  NOTE! Unsynchronization is only supported for reading

*/

typedef unsigned long ulong;
typedef unsigned char uchar;

enum ID3_hdr_flag {
    UNSYNC = 0x80,
    XTND   = 0x40,
    XPER   = 0x20
};

enum ID3_frm_flag1 {
    TAP    = 0x80,
    FAP    = 0x40,
    RO     = 0x20
};

enum ID3_frm_flag2 {
    PACK   = 0x80,
    ENC    = 0x40,
    GRP    = 0x20
};

struct raw_hdr {
    uchar ID   [3];
    uchar ver;
    uchar rev;
    uchar flags;
    uchar size [4];
};

struct raw_frm {
    uchar ID   [4];
    uchar size [4];
    uchar flags[2];
};

typedef raw_hdr_size_check [sizeof(struct raw_hdr)==10 ? 1 : -1];
typedef raw_frm_size_check [sizeof(struct raw_frm)==10 ? 1 : -1];

/* ==================================================== */

 /* en-/de- unsynchronizing */

static uchar *unsync_dec(uchar *buf, ulong size)
{
    uchar *dst, *src;

    dst = src = buf;                          /* in-place de-unsync'ing */
    while(size--)
        if( (*dst++ = *src++) == 0xFF && size > 0 ) {
            size--;
            if( (*dst = *src++) != 0x00 ) dst++;
        }
    return dst;
}

static uchar *unsync_enc(uchar *buf, ulong size)
{
    uchar *dst, *src;

    dst = src = buf;                          /* in-place en-unsync'ing */
    while(size--);

     /* to be 'ritten ;) */

}

/* ==================================================== */

static ulong ul4(uchar n[4])
{
    return (ulong)n[0]<<24 |
           (ulong)n[1]<<16 |
           (ulong)n[2]<< 8 |
           (ulong)n[3]<< 0 ;
}

static void nbo4(uchar h[4], ulong n)
{
    h[0] = (n >> 24);
    h[1] = (n >> 16);
    h[2] = (n >>  8);
    h[3] = (n      );
}

static void setsize(struct raw_hdr *h, ulong size)
{
    h->size[0] = (size >> 21) & 0x7F;
    h->size[1] = (size >> 14) & 0x7F;
    h->size[2] = (size >>  7) & 0x7F;
    h->size[3] = (size      ) & 0x7F;
}

static ulong getsize(struct raw_hdr *h)
{
    return (h->size[0] & 0x7F) << 21
         | (h->size[1] & 0x7F) << 14
         | (h->size[2] & 0x7F) <<  7
         | (h->size[3] & 0x7F);
}

static ulong calcsize(uchar *buf, ulong max)
{
    struct raw_frm *frame;
    ulong size = 0;
    ulong step;

    while(size < max && *buf) {            /* terminates if used properly */
        frame = (struct raw_frm*)buf;
        step  = sizeof(*frame) + ul4(frame->size);
        size += step;
        buf  += step;
    }
    return size<=max? size : 0;
}

static int checkid(const char ID[])
{
    return (isupper(ID[0]) || isdigit(ID[0]))
        && (isupper(ID[1]) || isdigit(ID[1]))
        && (isupper(ID[2]) || isdigit(ID[2]))
        && (isupper(ID[3]) || isdigit(ID[3]));
}

/* ==================================================== */

void *ID3_readf(const char *fname, size_t *tagsize)
{
    struct raw_hdr rh;
    uchar *buf;
    ulong size, pad;

    FILE *f = fopen(fname, "rb");

    if(tagsize) *tagsize = 0;                                   /* clear */

    if( !f ) return 0;

    if( fread(&rh, sizeof(struct raw_hdr), 1, f) != 1 )
        goto abort;                                          /* IO error */

    if( memcmp(rh.ID, "ID3", 3) != 0 || rh.ver != 3 )
        goto abort;                                /* not an ID3v2.3 tag */

    size = getsize(&rh);

    buf = malloc(size+4);                          /* over-alloc 4 chars */
    if(!buf)                                         /* ohhhhhhh.. crap. */
        goto abort_mem;

    buf[size] = 0;       /* make sure we have a pseudoframe to terminate */

    if( fread(buf,size,1,f) != 1 )
        goto abort_mem;                            /* IO error part deux */

    if( rh.flags & UNSYNC )
        size = unsync_dec(buf, size) - buf;

    if( rh.flags & XTND ) {                 /* get rid of extended header */
        ulong xsiz = ul4(buf);
        size -= xsiz;
        memmove(&buf[0], &buf[xsiz], size);
    }

    pad  = size;                                /* check semantics of tag */
    size = calcsize(buf, size);
    if(tagsize) *tagsize = size;

    if(size == 0)                                /* semantic error in tag */
        goto abort_mem;

    while(size < pad) {
        if( buf[size++] != 0 )                        /* padding not zero */
            goto abort_mem;
    }

    fclose(f);
    return buf;

abort_mem:                     /* de-alloc, close file and return failure */
    if(tagsize) *tagsize = (size_t)-1;                  /* evil ID3v2 tag */
    free(buf);
abort:                                   /* close file and return failure */
    fclose(f);
    return 0;
}

int (*ID3_wfail)(const char *srcname, const char *dstname) = cpfile;

int ID3_writef(const char *fname, void *src, size_t reqsize)
{
    struct raw_hdr new_h = { "ID3", 3, 0, 0, { 0, } };
    struct raw_hdr rh    = { { 0 } };                       /* duct tape */
    ulong size = calcsize(src,ULONG_MAX);

    FILE *f = fopen(fname, "rb+");

    if(!f) return 0;

    fread(&rh, sizeof(struct raw_hdr), 1, f);

    if( memcmp(rh.ID, "ID3", 3) == 0 ) {             /* allready tagged? */
        ulong orig;

        if( rh.ver != 3 )
            goto abort;                          /* only handles ID3v2.3 */

        orig = getsize(&rh);

        if( size>0 && size<=orig && !reqsize) { /* enough reserved space */
            setsize(&new_h, orig);
            rewind(f);
            fwrite(&new_h, sizeof new_h, 1, f);   /* i don't check these */
            fwrite(src, size, 1, f);
            fpadd(f, 0, orig-size);
            fclose(f);
            return 1;
        }
        fseek(f, orig, SEEK_CUR);
    } else {
        if(size == 0) {
            fclose(f);
            return 1;
        }
        rewind(f);
    }
                                                        /* file rewriter */
    {              
        ulong nsize = ((size+sizeof new_h+0x1FF) & ~0x1FF) - sizeof new_h;
        int ok;                                      /* rnd to 512 bytes */

        char *tmp;
        FILE *nf = opentemp(fname, &tmp);

        if( reqsize ) {
            reqsize = (reqsize < sizeof new_h)? 0 : reqsize - sizeof new_h;
            nsize   = (size < reqsize)? reqsize : size;
        }

        if( !nf )
            goto abort;

        if(size != 0) {
            setsize(&new_h, nsize);
            ok = fwrite(&new_h, sizeof new_h, 1, nf) == 1
              && fwrite(src, size, 1, nf)            == 1
              && fpadd(nf, 0, nsize-size)
              && fcopy(nf, f);
        } else {
            ok = fcopy(nf, f);                  /* remove ID3v2 tag only */
        }
        fclose(f);
        fclose(nf);

        if(ok && remove(fname) == 0) {
            if( rename(tmp, fname) != 0 && !ID3_wfail(tmp, fname) ) {
                fprintf(stderr, "%s -> %s: %s\n", tmp, fname, strerror(errno));
                exit(255);                                  /* sayonara! */
            }
        } else {
            remove(tmp);                                      /* failure */
            free(tmp);
            return 0;
        }
        free(tmp);
    }
    return 1;

abort:                                  /* close file and return failure */
    fclose(f);
    return 0;
}

void ID3_free(void *buf)
{
    free(buf);
}

/* ==================================================== */

void ID3_start(ID3FRAME f, void *buf)
{
    f->ID[4] = 0;
    f->data = buf;
    f->size = 0;
}

int ID3_frame(ID3FRAME f)
{
    struct raw_frm *frame;

    f->data += f->size + sizeof *frame;
    frame = (struct raw_frm*)f->data - 1;

    f->size = ul4(frame->size);                        /* copy essentials */
    memcpy(f->ID, frame->ID, 4);
    f->tag_volit  = frame->flags[0] & TAP;
    f->file_volit = frame->flags[0] & FAP;

    f->readonly   = frame->flags[0] & RO;
    f->packed     = frame->flags[1] & PACK;
    f->encrypted  = frame->flags[1] & ENC;
    f->grouped    = frame->flags[1] & GRP;

    return checkid(f->ID);
}

/* ==================================================== */

void *ID3_put(void *dest, const char ID[4], const void *src, size_t len)
{
    struct raw_frm *frame = (struct raw_frm*)dest;
    char *cdest           = (char*)dest + sizeof *frame;

    if(!ID || !checkid(ID))
        return (*(char*)dest=0), dest;

    memcpy(frame->ID, ID, 4);
    nbo4(frame->size, len);
    frame->flags[0] = 0;
    frame->flags[1] = 0;

    memcpy(cdest, src, len);
    cdest[len] = 0;                                        /* suffixing 0 */
    return cdest + len;
}

