#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#if defined(_WIN32)
#    include <io.h>
#    define F_OK 0
#else
#    include <unistd.h>
#endif
#include "id3v2.h"

#define NAME "id3-images"

/*

  minimalistic program to extract cover art situated in ID3v2 tags

  copyright (c) 2015 Marc R. Schoolderman <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

static const char* picture_types[] = {
    "other",
    "icon",
    "other_icon",
    "front_cover",
    "back_cover",
    "leaflet",
    "media",
    "lead_artist",
    "artist",
    "conductor",
    "band",
    "composer",
    "lyricist",
    "location",
    "recording",
    "performance",
    "screencap",
    "red_herring",
    "illustration",
    "logotype",
    "studio_logotype"
};

static void Help(const char *name)
{
    const char *base = strchr(name, '/');
    printf(
        NAME " 0.2\n"
        "Extract embedded art from ID3v2 tags to current directory\n"
        "usage: %s filename.mp3\n"
        "\nReport bugs to <squell@alumina.nl>.\n",
        base? base+1 : name
    );
    exit(1);
}

static void eprintf(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    fprintf (stderr, "%s: ", NAME);
    vfprintf (stderr, msg, args);
    va_end(args);
}

const char* mime_ext(const char *fname, const char* mime_type)
{
    static const char* exts[] = {
        "image/jpeg", ".jpg",
        "image/png", ".png",
        0
    };
    size_t i;
    for(i=0; exts[i]; i+=2) {
        if(strcmp(mime_type, exts[i]) == 0)
            return exts[i+1];
    }
    eprintf("%s: unknown mime type: %s\n", fname, mime_type);
    return ".unknown";
}

void write_blob(const char *basename, const char* ext, const void* blob, size_t size)
{
    static char image_fn[512];
    FILE *f;

    strncpy(image_fn, basename, 510);
    strncat(image_fn, ext, 510);

    if( access(image_fn, F_OK) == 0 ) {
        eprintf("`%s' already exists, not overwriting\n", image_fn);
        return;
    }

    f = fopen(image_fn, "wb");
    if(fwrite(blob, 1, size, f) != size | fclose(f) != 0) {
        eprintf("`%s' could not be written\n", image_fn);
        perror(0);
    } else {
        printf("%s\n", image_fn);
    }
}

const char *membrk0(const char *buf, size_t size, int wide)
{
    const char* const end = buf + size - wide;
    const int step = 1+wide;
    for( ; buf < end; buf += step) {
        if(!buf[0] && !buf[wide])
            return buf;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc <= 1 || argc > 2) Help(argv[0]);

    if(*++argv) {
        void *tag = ID3_readf(*argv, 0);
        ID3FRAME f;
        int counter = 0;

        if(!tag) return 0;

        if(ID3_start(f,tag) >= 2) {
            while(ID3_frame(f)) {
                if(strcmp(f->ID, "APIC") == 0) {

                    /* see ID3v2.3 4.15 -- 'Attached Picture' for reference */

                    char wide = f->data[0] == 1 || f->data[0] == 2;
                    const char *mime_type = f->data+1;
                    const char *type = memchr(mime_type, 0, f->size-(2+wide));
                    const char *descr, *blob;

                    if(!type || (type[1]&0xFFu) > sizeof picture_types/sizeof *picture_types) {
                        eprintf("%s has an incorrect ID3v2 tag!\n", *argv);
                        continue;
                    } else {
                        ++type;          /* skip terminator */
                    }

                    descr = type+1;
                    blob = membrk0(descr, f->size-(descr-f->data), wide);
                    if(!blob) {
                        eprintf("%s has an incorrect ID3v2 tag!\n", *argv);
                        continue;
                    } else {
                        blob += 1+wide;  /* skip terminator */
                    }

                    write_blob( picture_types[*type],
                                mime_ext(*argv, mime_type),
                                blob,
                                f->size - (blob - f->data) );
                    counter++;
                }
            }
        }

        if(counter == 0)
            eprintf("%s contains no embedded images\n", *argv);
        ID3_free(tag);
    }
    return 0;
}

