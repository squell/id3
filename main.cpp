#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include "setid3.h"
#include "varexp.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

void write_mp3s(const char* fspec, smartID3& tag)
{
    char path[sizeof dirent().d_name * 2];
    strncpy(path, fspec, sizeof path);          // copy constant
    path[sizeof path-1] = 0;                    // duct tape

    char* pname = strrchr(path, '/');           // pos to append filename to
    if(pname) {
        *(++pname) = 0;                         // seperate path & fspec
        fspec += (pname-path);                  // increase expression ptr
    } else {
        pname = strcpy(path, "./");
    }

    DIR* dir = opendir(path);
    if(!dir)
        return (void) printf("err: could not read %s\n", path);

    bool m = false;                             // idle warning check

    while( dirent* fn = readdir(dir) ) {
        varexp match(fspec, fn->d_name);
        strcpy(pname, fn->d_name);
        if( match && ++m && !tag.modify(path, match) )
            printf("err: could not edit %s!\n", fn->d_name);
    }
    closedir(dir);

    if(!m)
        printf("err: no files matching %s\n", fspec);
}

/* ====================================================== */

void help(const char* argv0)
{
    printf(
        "usage: %s [OPTIONS] filespec ...\n\n"
        " -d \tclear existing tag\n"
        " -t <title>\n"
        " -a <artist>\n"
        " -l <album>\n"
        " -y <year>\n"
        " -c <comment>\n"
        " -g <genre>\n"
        " -n <track>\n"
        "\tset ID3 fields\n\n"
        "Any occurences of the form \"%%i\" in an ID3 field value will be substituted by\n"
        "the portion of the actual filename matched by the i'th \"*\" wildcard, where i\n"
        "is a digit in the range [1..9,0].\n",
        argv0
    );
    exit(0);
}

int main(int argc, char *argv[])
{
    if(argc <= 1) help(argv[0]);

    bool   w = false;            // check against no-ops, not really needed
    ID3set t = ID3;
    smartID3 tag;

    for(int i=1; i < argc; i++) {
        if( argv[i][0] != '-' )
            if(t != ID3) {
                tag.set(t, argv[i]);
                t = ID3;
                w = true;
            } else {
                if(w)
                {
                    try{
                        write_mp3s(argv[i], tag);
                    } catch(const out_of_range& x) {
                        printf("err: wildcard index out of range\n");
                    }
                }
                else
                    printf("err: nothing to do with %s\n", argv[i]);
            }
        else
            switch( toupper(argv[i][1]) ) {
            case 'D': tag.clear(); w = true; break;
            case 'T': t = title;  break;
            case 'A': t = artist; break;
            case 'L': t = album;  break;
            case 'Y': t = year;   break;
            case 'C': t = cmnt;   break;
            case 'G': t = genre;  break;
            case 'N': t = track;  break;
            default:
                printf("err: unrecognized switch: -%c\n", argv[i][1]);
                exit(1);
            }
    }
}

