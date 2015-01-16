#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "fileops.h"
#include "id3v2.h"

/*

  copyright (c) 2003-2006 squell <squell@alumina.nl>

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

static long getsize(struct raw_hdr *h)
{
    return (h->size[0] & 0x7F) << 21
         | (h->size[1] & 0x7F) << 14
         | (h->size[2] & 0x7F) <<  7
         | (h->size[3] & 0x7F);
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

    while(size < max && checkid(buf,1+version)) {
        frame = (union raw_frm*)buf;
        switch(version) {
            case  2: step = sizeof(frame->v2) + (ul4(frame->v2.size) >> 8); break;
            case  3: step = sizeof(frame->v3) + ul4(frame->v3.size);        break;
            default: return -1;
        }
        size += step;
        buf  += step;
    }
    return size<=max? size : -1;
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
	refuse(abort_file, "file too small", 0);             /* IO error */

    if( memcmp(rh.ID, "ID3", 3) != 0 )               /* not an ID3v2 tag */
	refuse(abort_file, "contains no ID3 identifier", 0); 
    if( (rh.ver|1) != 3 )			      /* unknown version */
	refuse(abort_file, "unsupported ID3v2.%d", rh.ver);

    size = getsize(&rh);

    buf = malloc(size+1+4);                      /* over-alloc 4+1 chars */
    if(!buf)                                         /* ohhhhhhh.. crap. */
	refuse(abort_file, "could not allocate tag (%ld bytes)", size);

    (++buf)[-1] = rh.ver;                        /* prepend version byte */
    buf[size] = 0;       /* make sure we have a pseudoframe to terminate */

    if( fread(buf,size,1,f) != 1 )
	refuse(abort_mem, "could not read tag from file (%ld bytes)", size);

    if( rh.flags & UNSYNC )
        size = unsync_dec(buf, size) - buf;

    if( rh.flags & XTND ) {                 /* get rid of extended header */
        ulong xsiz = ul4(buf) + 4;      /* note: compression bit in v2.2, */
	if(xsiz >= size)
	    refuse(abort_mem, "extended header incorrect (%ld bytes)", xsiz);
        size -= xsiz;                                   /* but try anyway */
        memmove(&buf[0], &buf[xsiz], size);
    }

    pad  = size;                                /* check semantics of tag */
    size = calcsize(buf, size);
    if(tagsize) *tagsize = size;

    if(size < 0)                                 /* semantic error in tag */
        refuse(abort_mem, "tag larger than reported size (%ld bytes)", pad);

    while(size < pad) {
        if( buf[size++] != 0 )                        /* padding not zero */
	    refuse(abort_mem, "padding contains non-null data (%02x)", buf[size-1]);
    }

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

        if( (rh.ver|1) != 3)
            goto abort;                       /* handles ID3v2.2 and 2.3 */

        orig = getsize(&rh);

        if( size>0 && size<=orig && !reqsize) { /* enough reserved space */
            setsize(&new_h, orig);
            rewind(f);
            fwrite(&new_h, sizeof new_h, 1, f);   /* i don't check these */
            fwrite(src, size, 1, f);
            fpadd(0, orig-size, f);
            fclose(f);
            return 1;
        }
        if( fseek(f, orig, SEEK_CUR) != 0 )
            goto abort;
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
            ok = fwrite(&new_h, 1, sizeof new_h, nf) == sizeof new_h
              && fwrite(src, 1, size, nf)            == size
              && fpadd(0, nsize-size, nf)            == nsize-size
              && fcopy(nf, f);
        } else {
            ok = fcopy(nf, f);                  /* remove ID3v2 tag only */
        }
        fclose(f);
        fclose(nf);

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
    f->ID[3] = ver == 3;                 /* set to indicate tag version */
    f->ID[4] = 0;
    f->data  = (char*)buf + 1;
    f->size  = 0;

    f->tag_volit  =                      /* set highly useful data */
    f->file_volit =
    f->readonly   =
    f->packed     =
    f->encrypted  =
    f->grouped    = 0;

    return (ver|1) == 3 ? ver : 0;
}

int ID3_frame(ID3FRAME f)
{
    union raw_frm *frame = (union raw_frm*)(f->data + f->size);
    int ver = !!f->ID[3];

    f->data += f->size + raw_frm_sizeof[ver];

    memcpy(f->ID, frame->ID, 3+ver);

    if(ver) {                                           /* ID3v2.3 stuff */
        f->size       = ul4(frame->v3.size);          /* copy essentials */
        f->tag_volit  = !!( frame->v3.flags[0] & TAP  );
        f->file_volit = !!( frame->v3.flags[0] & FAP  );

        f->readonly   = !!( frame->v3.flags[0] & RO   );
        f->packed     = !!( frame->v3.flags[1] & PACK );
        f->encrypted  = !!( frame->v3.flags[1] & ENC  );
        f->grouped    = !!( frame->v3.flags[1] & GRP  );
    } else {
        f->size       = ul4(frame->v2.size) >> 8;
    }

    return checkid(f->ID, 3+ver);
}

/* ==================================================== */

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

