#include <stdio.h>
#include <string.h>

  This file needs work. It will be included in `id3' in the future, and
  is provided only for completeness.

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  /*

  Most of this code was based on information available from:

   * http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm    (mp3 frame info)
   * http://www.thecodeproject.com/audio/MPEGAudioInfo.asp (vbr headers)
   * http://gabriel.mp3-tech.org/mp3infotag.htm            (lame header)

  To do;
  - testing all possibile combinations of hz/bitrate, header/scan
  - make sure hz and bitrate stays the same during one mp3
  - analyse how malicious mp3's are handled
  - (not in this file) figure out syntax for variable use

   %m min       ?
   %s sec
   %r rate
   %b bitrate

  */

#define MAX_SYNC    0xC000U

typedef struct mp3_info {
    signed   long length;
    unsigned long samples;
    unsigned long octets;
    unsigned bitrate;
    unsigned hz;
    char     header;
    char     vbr;         // not used
    enum {
        MP3_STEREO,
        MP3_JS,
        MP3_DUAL,
        MP3_MONO,
    } mode;
} MP3_INFO;

#define UL4(p) \
 (((p)[0] & 0xFFUL) << 24 | ((p)[1] & 0xFFUL) << 16 | \
  ((p)[2] & 0xFFUL) <<  8 | ((p)[3] & 0xFFUL))

#ifdef DEBUG
#    define DEBUG_log(s) printf(s)
#else
#    define DEBUG_log(s) 0
#endif

static unsigned long remain(FILE *f)
{
    unsigned long t = ftell(f);
    fseek(f, 0, SEEK_END);
    return ftell(f) - t;
}

 /*

  Ooooh! goto statements! Lots of em too! And flipped bits.

  Trig: (-id>>1)&1 == !!id for bit values 0..2

  id = 0 (MPEG1), id = 1 (MPEG2), id = 2 (MPEG 2.5)

  The 'accurate division' is identical to a basic divide when hz rate is
  a multiple of 1000. It has to be done because multiplying the number of
  samples and then dividing might overflow a 32bit ulong for even very
  moderately sized files. I somehow dislike doing floating point math.

  */

struct mp3_info *mp3_getinfo(const char *name, int scan, struct mp3_info *report)
{
    FILE *f = fopen(name, "rb");
    unsigned char buf[0x4000];
    unsigned char hdr[1441];                        /* max frame size */

    const static unsigned bps[2][16] = {            /* look up table */
      { 320, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, },
      { 160,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160, }
    };

    const static unsigned srate[] = {
        44100, 48000, 32000
    };

    unsigned id, bitrate, freq, pad, mode, crc;     /* mp3 frame info */

    unsigned long sum = 0, siz = 0;
    int c, n, didframe = 0, err = 0;

    report->length = -1;

    if(!f) return 0;                                /* file not open ? */

    setvbuf(f, buf, _IOFBF, sizeof buf);

sync:
    while((c=getc(f)) != EOF) {
        if(c == 0xFF) goto frame;                   /* possible frame sync */
        if(++err > MAX_SYNC)
            return (void)fclose(f), report;
    }

end:
#ifdef DEBUG
    printf("\n");
    if(err)
        printf("skipped %d bytes\n", err);
    printf("%d frames\n", sum);
#endif
    if(sum) {
        unsigned long divisor, tmp;
        report->hz      = srate[freq] >> id;
        report->mode    = mode;
        report->bitrate = bps[!!id][bitrate];
        report->samples = sum*(1152UL >> !!id);
        report->octets  = siz;
        divisor = report->hz / 1000;                /* lossless division */
        report->length  = report->samples / divisor;
        tmp = (report->hz%1000) * report->length - (report->samples%divisor);
        report->length -= tmp / report->hz;
    }
    fclose(f);
    return report;

frame:
    DEBUG_log("\n");
    c = getc(f);
    id  = c >> 3 & 0x3;                             /* 3=mp1 2=mp2 0=mp2.5 */
    id  = id ^ id>>1 ^ 0x2;                         /* 0=mp1 1=mp2 2=mp2.5 */
    crc = c & 0x01;
    if((c & 0xE6) != 0xE2 || id == 3) {             /* only support layer3 */
#ifdef DEBUG
        if((c & 0xE6) != 0xE0 || id == 3 || (c & 0x6) == 0)
            printf("phony sync");
        else
            printf("wrong: layer %d", 4-((c&6)>>1));
#endif
        err += 2;
        goto sync;
    }

    c = getc(f);
    bitrate = c >> 4 & 0xF;
    freq    = c >> 2 & 0x3;
    pad     = c >> 1 & 0x1;
    if(bitrate == 0xF || freq == 0x3) {
        DEBUG_log("error in attributes - resync");
        err += 3;
        goto sync;
    }

    c = getc(f);
    mode = c >> 6 & 0x3;

    switch( freq ) {                                /* divide by constants */
    case 0:                                         /* for compiler opts   */
        n = (bps[(-id>>1)&1][bitrate] << (id>>1))*1440UL/441UL + pad;
        break;
    case 1:
        n = (bps[(-id>>1)&1][bitrate] << (id>>1))*1440UL/480UL + pad;
        break;
    case 2:
        n = (bps[(-id>>1)&1][bitrate] << (id>>1))*1440UL/320UL + pad;
        break;
    }
    err = 0;                                        /* successful frame */

#ifdef DEBUG
    if(err)
       printf("skipped %d bytes\n", err);
    printf("* %dkbit %dhz %d octets", bps[!!id][bitrate], srate[freq] >> id, n + pad);
#endif

    if(! didframe ) {                               /* check for header */
        unsigned char *p = &hdr[2 + (30 >> (!!id + (mode==3)))];
        unsigned char *fcnt, *bcnt;
        DEBUG_log("\n // trying frame ");
        fcnt = bcnt = 0;
        if((didframe = fread(hdr, n-4, 1, f)) == 0) {
            DEBUG_log(" read error\n");
            goto sync;
        }
        if((strncmp(p, "Xing", 4) == 0 || strncmp(p, "Info", 4) == 0) && (p[7]&3) == 3) {
            report->header = *p;
            fcnt = &p[ 8];
            bcnt = &p[12];
#       ifdef DEBUG
            printf("%.4s %u frames, %u octets\n", p, UL4(fcnt), UL4(bcnt));
#       endif
        } else if(strncmp(&hdr[32], "VBRI", 4) ==0) {
            report->header = 'V';
            bcnt = &hdr[32+10];
            fcnt = &hdr[32+14];
#       ifdef DEBUG
            printf("VBRI %u frames, %u octets\n", UL4(fcnt), UL4(bcnt));
#       endif
        } else if(!scan) {
            DEBUG_log("- no header, extrapolating\n");
            report->hz      = srate[freq] >> id;    /* estimate length */
            report->mode    = mode;
            report->bitrate = bps[!!id][bitrate];
            report->octets  = remain(f) + n;
            report->length  = report->octets / (bps[!!id][bitrate] >> 3);
            goto end;
        } else {
            DEBUG_log("- no header\n");
            ++sum;                                  /* count frame */
            siz += n;
        }
        if(scan)
            goto sync;
        sum = UL4(fcnt);
        siz = UL4(bcnt);
        goto end;
    }
    sum += fseek(f, n-4, SEEK_CUR) + 1;
    siz += n;
    goto sync;
}

const char* m[] = { "Stereo", "JointStereo", "Dual", "Mono" };

int main(int argc, char* argv[])
{
    int t = 0;
    if(*argv[1] && strcmp(argv[1], "+") == 0) { t++; argv++; }
    while(*++argv) {
        MP3_INFO i;
        if( mp3_getinfo(*argv, t, &i) ) {
            printf("%3d:%02d [%s]\n", i.length/60000, i.length/1000%60, *argv);
            printf("\t %3lukbit %5luhz %s MP3 avg: %lu\n", i.bitrate, i.hz, m[i.mode], ((i.octets<<3) + (i.length>>1)) / i.length);
            printf("\t %luocts %lusampls %lumsec\n", i.octets, i.samples, i.length);
        } else {
            printf("------ [%s] not an mp3!\n", *argv);
        }
    }
}
