#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fileops.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

int fcopy(FILE *dest, FILE *src)
{
    char buffer[0x4000];                              /* 16kb buffer */
    size_t r, w;

    do {
        r = fread (buffer, 1, sizeof buffer, src);
        w = fwrite(buffer, 1, r, dest);
        if(w != r) return 0;
    } while(r == sizeof buffer);

    return feof(src);
}

int fpadd(FILE *dest, char c, size_t len)
{
    char buffer[0x4000];                              /* 16kb buffer */
    size_t w;

    memset(buffer, c, sizeof buffer);

    while(len > sizeof buffer) {
        w = fwrite(buffer, sizeof buffer, 1, dest);
        if(w!=1) return 0;
        len -= sizeof buffer;
    }

    w = fwrite(buffer, 1, len, dest);

    return w == len;
}

char *tmpnam_alloc(const char *hint)
{
    char *buf;
#ifdef USE_TMPNAM
    if(buf = malloc(L_tmpnam)) {
        if(tmpnam(buf))
            return buf;
        free(buf);
    }
#else
    char* pname = strrchr(hint, '/');
    size_t idx  = pname? pname-hint+1 : 0;

    if(buf = malloc(idx + 8 + 1)) {
        strncpy(buf, hint, idx);
        strcpy (buf+idx, "idXXXXXX");
        if(mktemp(buf))
            return buf;
        free(buf);
    }
#endif
    return 0;
}

FILE *opentemp(const char *hint, char **name)            /* free() name! */
{
    char *buf;
    FILE *f;

    if(buf = tmpnam_alloc(hint)) {
        if(f = fopen(buf, "wb")) {
            if(name) *name = buf;
            else     free(buf);
            return f;
        }
        free(buf);
    }

    return 0;
}

/* ==================================================== */

int cpfile(const char *srcnam, const char *dstnam)
{
    FILE *src, *dst;
    int result = 0;

    if(src = fopen(srcnam, "rb")) {
        if(dst = fopen(dstnam, "wb")) {
            result = fcopy(dst, src);
            fclose(dst);
        }
        fclose(src);
    }
    return result;
}

