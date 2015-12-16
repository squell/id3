#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "fileops.h"
#include "id3v2.h"

/*

  copyright (c) 2003-2006, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  NOTE! Unsynchronization is only supported for reading

*/

/* some programs write evil tags. it's probably nice to see failure modes */
#ifdef ID3v2_DEBUG
#   define refuse(label, msg, info) { \
        fprintf(stderr, "%s: id3v2 "msg"\n", fname, info); \
        goto label; \
    }
#else
#   define refuse(label, msg, info) goto label;
#endif

/* fix bugs introduced by other programs? */
#define ID3v2_FIX 1

typedef unsigned long ulong;
typedef unsigned char uchar;

enum ID3_hdr_flag {
    UNSYNC = 0x80,
    XTND   = 0x40,      /* 2.3; in 2.2: unused compression bit */
    XPER   = 0x20,
    FOOTER = 0x10       /* 2.4 */
};

enum ID3_frm_flag1 {
    TAP    = 0x80,      /* 2.3, in 2.4: shifted 1 to the right */
    FAP    = 0x40,
    RO     = 0x20
};

enum ID3_frm_flag2 {
    PACK   = 0x80,      /* 2.3 */
    ENC    = 0x40,
    GRP    = 0x20,

    GRP4   = 0x40,      /* 2.4 */
    PACK4  = 0x08,
    ENC4   = 0x04,
    UNSYNC4= 0x02,
    DLI4   = 0x01
};

struct raw_hdr {
    uchar ID   [3];
    uchar ver;
    uchar rev;
    uchar flags;
    uchar size [4];
};

union raw_frm {
    uchar ID[4];
    struct raw_frm_2 {
        uchar ID   [3];
        uchar size [3];
    } v2;
    struct raw_frm_3 {
        uchar ID   [4];
        uchar size [4];
        uchar flags[2];
    } v3;
};

typedef int raw_hdr_size_check [sizeof(struct raw_hdr)==10 ? 1 : -1];
typedef int raw_frm_size_check [sizeof(union  raw_frm)==10 ? 1 : -1];

/* ==================================================== */

 /* en-/de- unsynchronizing */

static uchar *unsync_dec(uchar *dst, uchar *src, ulong size)
{
    while(size--)
        if( (*dst++ = *src++) == 0xFF && size > 0 ) {
            size--;
            if( (*dst = *src++) != 0x00 ) dst++;
        }
    return dst;
}

/* ==================================================== */

static ulong ul4(uchar n[4])
{
    return (ulong)n[0]<<24
         | (ulong)n[1]<<16
         | (ulong)n[2]<< 8
         | (ulong)n[3]<< 0;
}

static void nbo4(uchar h[4], ulong n)
{
    h[0] = (n >> 24) & 0xFF;
    h[1] = (n >> 16) & 0xFF;
    h[2] = (n >>  8) & 0xFF;
    h[3] = (n      ) & 0xFF;
}

static ulong ul4ss(uchar h[4])                            /* "synch safe" */
{
    return (ulong)(h[0] & 0x7F) << 21
         | (ulong)(h[1] & 0x7F) << 14
         | (ulong)(h[2] & 0x7F) <<  7
         | (ulong)(h[3] & 0x7F);
}

static void nbo4ss(uchar h[4], ulong n)
{
    h[0] = (n >> 21) & 0x7F;
    h[1] = (n >> 14) & 0x7F;
    h[2] = (n >>  7) & 0x7F;
    h[3] = (n      ) & 0x7F;
}

static int checkid(const char *ID, size_t n)    /* check ID for A..Z0..9 */
{
    while(n--) {
        if( !isupper(*ID) && !isdigit(*ID) ) return 0;
        ++ID;
    }
    return 1;
}

static long calcsize(uchar *buf, ulong max)
{
    union raw_frm *frame;
    ulong size = 0;
    ulong step;
    int version = buf[-1];
    int ID_siz = 3+(version>2);

    while(size < max && checkid((char*)buf, ID_siz)) {
        frame = (union raw_frm*)buf;
        switch(version) {
            case  2: step = sizeof(frame->v2) + (ul4(frame->v2.size) >> 8); break;
            case  3: step = sizeof(frame->v3) + ul4(frame->v3.size);        break;
            case  4: step = sizeof(frame->v3) + ul4ss(frame->v3.size);      break;
            default: return -1;
        }
        if(size+step <= size) return -1;
        size += step;
        buf  += step;
    }
    return size<=max? (long)size : -1;
}

/* in v2.4, unsync is per-frame for not adequately explained reasons.
   this function requires the actual size as determined by calcsize() */

static ulong unsync_frames_v2_4(uchar *buf, ulong size)
{
    uchar *end = buf+size;
    uchar *out = buf;

    while(buf < end) {
        union raw_frm *frame = (union raw_frm*)buf;
        ulong step = sizeof(frame->v3) + ul4ss(frame->v3.size);
        if( frame->v3.flags[1] & UNSYNC4 ) {
            frame = (union raw_frm*)out;
            out = unsync_dec(out, buf, step);
            /* update frame size & clear UNSYNC4 bit */
            nbo4ss(frame->v3.size, out-frame->ID - sizeof(*frame));
            frame->v3.flags[1] &= ~UNSYNC4;
        } else {
            out = (uchar*)memmove(out, buf, step) + step;
        }
        buf += step;
    }
    memset(out, 0, buf-out);
    return out - (end-size);
}

/* ==================================================== */

void *ID3_readf(const char *fname, size_t *tagsize)
{
    struct raw_hdr rh;
    uchar *buf;
    long pad, size = 0;

    FILE *f = fopen(fname, "rb");

    if( !f )
        refuse(abort, "could not open", 0);

    if( fread(&rh, sizeof(struct raw_hdr), 1, f) != 1 )
        refuse(abort_file, "file too small", 0);              /* IO error */

    if( memcmp(rh.ID, "ID3", 3) != 0 )                /* not an ID3v2 tag */
        refuse(abort_file, "contains no ID3 identifier", 0);

    if( rh.ver < 2 || rh.ver > 4 )                     /* unknown version */
        refuse(abort_file, "unsupported ID3v2.%d", rh.ver);

    size = ul4ss(rh.size);

    buf = malloc(size+1+4);                       /* over-alloc 4+1 chars */
    if(!buf)                                          /* ohhhhhhh.. crap. */
        refuse(abort_file, "could not allocate tag (%ld bytes)", size);

    (++buf)[-1] = rh.ver;                         /* prepend version byte */
    buf[size] = 0;        /* make sure we have a pseudoframe to terminate */

    if( fread(buf,1,size,f) != size )
        refuse(abort_mem, "could not read tag from file (%ld bytes)", size);

    if( rh.flags & UNSYNC && rh.ver <= 3 )        /* unsync on entire tag */
        size = unsync_dec(buf, buf, size) - buf;

    if( rh.flags & XTND ) {                 /* get rid of extended header */
        ulong xsiz = (rh.ver==3?ul4:ul4ss)(buf) + 4;
        if(xsiz < 4 || xsiz >= size)
            refuse(abort_mem, "extended header incorrect (%ld bytes)", xsiz);
        size -= xsiz;                                   /* but try anyway */
        memmove(&buf[0], &buf[xsiz], size);
    }

    pad  = size;                                /* check semantics of tag */
    size = calcsize(buf, size);

    if( rh.ver == 4 && size > 0 )  /* v2.4: just ignore the global UNSYNC */
        size = unsync_frames_v2_4(buf, size);

    if(tagsize) *tagsize = size;

    if(size < 0)                                 /* semantic error in tag */
        refuse(abort_mem, "tag larger than reported size (%ld bytes)", pad);

    while(size < pad)
        if( buf[size++] == 0xff ) {                   /* padding not zero */
            if(size-1 == pad-sizeof(struct raw_hdr) && ID3v2_FIX)
                ;           /* tag contains a rare bug; make an exception */
            else
                refuse(abort_mem, "padding contains framesync (%02x)", buf[size-1]);
        }

    ;                       /* nothing required to handle ID3v2.4 footers */

    fclose(f);
    return --buf;

abort_mem:                     /* de-alloc, close file and return failure */
    free(--buf);
abort_file:                              /* close file and return failure */
    fclose(f);
abort:
    if(tagsize) *tagsize = size;
    return 0;
}

#ifndef ID3v2_READONLY

static void _wfail(const char *srcname, const char *dstname)
{
    fprintf(stderr, "%s -> %s: %s\n", srcname, dstname, strerror(errno));
    exit(255);                                              /* sayonara! */
}

void (*ID3_wfail)(const char *srcname, const char *dstname) = _wfail;

int ID3_writef(const char *fname, const void *buf, size_t reqsize)
{
    struct raw_hdr new_h = { "ID3", 0, 0, 0, { 0, } };
    struct raw_hdr rh    = { { 0 } };                       /* duct tape */
    uchar* src;
    long size = 0;

    FILE *f = fopen(fname, "rb+");
    if(!f) return 0;

    if(buf) {
        src  = (uchar*)buf + 1;
        size = calcsize(src, LONG_MAX);
        new_h.ver = src[-1];
    }

    if(size < 0)
        goto abort;                                   /* error in caller */

    if( fread(&rh, sizeof(struct raw_hdr), 1, f) && memcmp(rh.ID, "ID3", 3) == 0 ) {
        long orig;                                   /* allready tagged? */

        if( rh.ver < 2 || rh.ver > 4 )
            goto abort;                           /* handles ID3v2.[234] */

        orig = ul4ss(rh.size);

        if( fseek(f, orig, SEEK_CUR) != 0 )
            goto abort;

        if( ID3v2_FIX && fseek(f, -10, SEEK_CUR) == 0 ) {
            if(ungetc(getc(f), f) == 0xFF)        /* fix off-by-10 error */
                orig -= 10;
            else
                if( fseek(f, 10, SEEK_CUR) != 0 ) goto abort;
        }

        if( size>0 && size<=orig && !reqsize) { /* enough reserved space */
            nbo4ss(new_h.size, orig);
            rewind(f);
            fwrite(&new_h, sizeof new_h, 1, f);
            fwrite(src, size, 1, f);
            fpadd(0, orig-size, f);
            if(ferror(f) | fclose(f)) {         /* abandon all hope,     */
                ID3_wfail(fname, fname);            /* ye who enter here */
                return 0;
            }
            return 1;
        }
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
            nbo4ss(new_h.size, nsize);
            ok = fwrite(&new_h, 1, sizeof new_h, nf) == sizeof new_h
              && fwrite(src, 1, size, nf)            == size
              && fpadd(0, nsize-size, nf)            == nsize-size
              && fcopy(nf, f);
        } else {
            ok = fcopy(nf, f);                  /* remove ID3v2 tag only */
        }
        fclose(f);
        ok = fclose(nf) == 0 && ok;

        if(ok) {
            ok = mvfile(tmp, fname);
            if(!ok)
                ID3_wfail(tmp, fname);                 /* grievous error */
        } else {
            remove(tmp);                                      /* failure */
        }
        free(tmp);
        return ok;
    }

abort:                                  /* close file and return failure */
    fclose(f);
    return 0;
}

#endif

void ID3_free(const void *buf)
{
    free((void*)buf);
}

/* ==================================================== */

static const size_t raw_frm_sizeof[2]
  = { sizeof(struct raw_frm_2), sizeof(struct raw_frm_3) };

ID3VER ID3_start(ID3FRAME f, const void *buf)
{
    register uchar ver = *(uchar*)buf;
    f->_rev  = ver-2;
    f->ID[3] = f->ID[4] = 0;
    f->data  = (char*)buf + 1;
    f->size  = 0;

    f->tag_volit  =                      /* set highly useful data */
    f->file_volit =
    f->readonly   =
    f->packed     =
    f->encrypted  =
    f->grouped    = 0;

    return ver>=2 && ver<=4? 2+(ver>2) : 0; /* pretend v2.4 = v2.3 */
}

int ID3_frame(ID3FRAME f)
{
    union raw_frm *frame = (union raw_frm*)(f->data + f->size);
    int version = f->_rev+2;
    int ID_siz = 3+(version>2);

    f->data += f->size + raw_frm_sizeof[version>2];

    memcpy(f->ID, frame->ID, ID_siz);

    if(version==3) {                                    /* ID3v2.3 stuff */
        f->size       = ul4(frame->v3.size);          /* copy essentials */
        f->tag_volit  = !!( frame->v3.flags[0] & TAP  );
        f->file_volit = !!( frame->v3.flags[0] & FAP  );
        f->readonly   = !!( frame->v3.flags[0] & RO   );

        f->packed     = !!( frame->v3.flags[1] & PACK );
        f->encrypted  = !!( frame->v3.flags[1] & ENC  );
        f->grouped    = !!( frame->v3.flags[1] & GRP  );
    } else if(version==4) {
        f->size       = ul4ss(frame->v3.size);
        f->tag_volit  = !!( frame->v3.flags[0]>>1 & TAP  );
        f->file_volit = !!( frame->v3.flags[0]>>1 & FAP  );
        f->readonly   = !!( frame->v3.flags[0]>>1 & RO   );

        f->packed     = !!( frame->v3.flags[1] & PACK4 );
        f->encrypted  = !!( frame->v3.flags[1] & ENC4  );
        f->grouped    = !!( frame->v3.flags[1] & GRP4  );

        if( frame->v3.flags[1] & DLI4 )          /* id3v2.4 crufty stuff */
            f->data += 4, f->size -= 4;
    } else {
        f->size       = ul4(frame->v2.size) >> 8;
    }

    return checkid(f->ID, ID_siz);
}

/* ==================================================== */

#ifndef ID3v2_READONLY

void *ID3_put(void *dest, ID3VER version, const char ID[4], const void *src, size_t len)
{
    union raw_frm *frame = (union raw_frm*)dest;
    uchar *cdest         = dest;

    if(!ID) {
        (++cdest)[-1] = version;                         /* initialize */
        cdest[0]      = 0;
        return cdest;
    } else if((version|1) != 3 || !checkid(ID, version+1) || ID[version+1]) {
        return cdest;
    }

    memcpy(frame->ID, ID, version+1);
    if(version == 3) {                                   /* ID3v2.3 stuff */
        nbo4(frame->v3.size, len);
        frame->v3.flags[0] = 0;
        frame->v3.flags[1] = 0;
    } else {
        nbo4(frame->v2.size, len << 8);      /* extra byte doesn't matter */
    }

    cdest = memcpy(cdest + raw_frm_sizeof[version==3], src, len);
    cdest[len] = 0;                                        /* suffixing 0 */
    return cdest + len;
}

#endif
