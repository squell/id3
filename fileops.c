#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fileops.h"
#include <sys/stat.h>
#if defined(_MSC_VER)
#    include <sys/utime.h>
#else
#    include <utime.h>
#endif
#if !defined(_WIN32)
#    include <unistd.h>
#else
#    include <io.h>
#endif
#if !defined(S_ISLNK)
#    define  lstat      (stat)
#    define  S_ISLNK(m) (0)
#endif

/*

  copyright (c) 2003, 2005, 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

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

size_t fpadd(char c, size_t len, FILE *dest)
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

    return w;
}

FILE *ftemp(char *templ, const char *mode)
{
    FILE *f;
#if defined(__DJGPP__) || defined(_WIN32)
    FILE *fc;
    if(mktemp(templ) && (fc = fopen(templ, "wb+"))) {
        if(f = freopen(templ, mode, fc)) return f;
        fclose(fc);
#else
    int fd = mkstemp(templ);
    if(fd >= 0) {
        FILE* fdopen();                         /* in case -ansi is used */
        if(f = fdopen(fd, mode)) return f;
        close(fd);
#endif
        unlink(templ);
    }
    return 0;
}

FILE *opentemp(const char *hint, char **name)            /* free() name! */
{
    static const char template[] = "idXXXXXX";

    char *buf, *dirsep = strrchr(hint, '/');
    size_t prefix = dirsep? dirsep-hint+1 : 0;
    FILE *f;

    if(buf = malloc(prefix + sizeof template)) {
        strncpy(buf, hint, prefix);
        strncpy(buf+prefix, template, sizeof template);
        if(f = ftemp(buf, "wb")) {
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
            if(fclose(dst) != 0) result = 0;
        }
        fclose(src);
    }
    return result;
}

int mvfile(const char *srcnam, const char *dstnam)
{
    struct stat fs;
    int file = 0, slow = 0;

    if(lstat(dstnam, &fs) == 0) {
        file = !S_ISLNK(fs.st_mode);
        slow = fs.st_nlink > 1 || !file;                   /* honour links */
        slow = slow || remove(dstnam) != 0;
    }
    if(slow || rename(srcnam, dstnam) != 0) {
        struct utimbuf buf, *stamp = 0;
        struct stat ss;
        if(stat(srcnam, &ss) == 0) {
            stamp       = &buf;                      /* preserve file time */
            buf.actime  = ss.st_atime;
            buf.modtime = ss.st_mtime;
        }
        if(!cpfile(srcnam, dstnam))
            return 0;        /* could not rename, could not copy - give up */
        (void) remove(srcnam);                        /* dont check result */
        (void) utime(dstnam, stamp);
    }
    if(file)
         (void) chmod(dstnam, fs.st_mode);
    return 1;                               /* successful rename (or copy) */
}

