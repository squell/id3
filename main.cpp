#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include "varexp.h"
#ifdef NO_V2
#  include "setid3.h"
#else
#  include "setid3v2.h"
#endif

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
            printf("err: could not access %s!\n", fn->d_name);
    }
    closedir(dir);

    if(!m)
        printf("err: no files matching %s\n", fspec);
}

/* ====================================================== */

void help(const char* argv0)
{
    printf(
#ifdef __ZF_SETID3V2
        "usage: %s [-1 -2] [OPTIONS] filespec ...\n\n"
#else
        "usage: %s [OPTIONS] filespec ...\n\n"
#endif
        " -d\tclear existing tag\n"
        " -t <title>\n"
        " -a <artist>\n"
        " -l <album>\n"
        " -y <year>\n"
        " -c <comment>\n"
        " -g <genre>\n"
        " -n <track>\n"
        "\tset ID3 fields\n"
#ifdef __ZF_SETID3V2
        "\nonly when -2:\n"
        " -dXXXX\terase all XXXX frames\n"
        " -wXXXX <data>\n\tdirectwrite an XXXX frame\n"
#endif
        "\nAny occurences of the form \"%%i\" in an ID3 field value will be substituted by\n"
        "the portion of the actual filename matched by the i'th \"*\" wildcard, where i\n"
        "is a digit in the range [1..9,0].\n",
        argv0
    );
    exit(0);
}

/* ====================================================== */

int main(int argc, char *argv[])
{
    ID3set t = ID3;
    bool   w = false;            // check against no-ops
#ifdef __ZF_SETID3V2
    bool aux = false;            // check for -1 & -2 commands
    string frmID;                // v2
    smartID3v2 tag(true,false);  // default: write ID3v1, not v2
#else
    smartID3 tag;
#endif

    for(int i=1; i < argc; i++) {
#ifdef __ZF_SETID3V2
        if(!frmID.empty()) {     // v2 - write raw frame
            tag.set(frmID, argv[i]);
            frmID.erase();
            w = true;
        } else
#endif
        if(t != ID3) {
            tag.set(t, argv[i]);
            t = ID3;
            w = true;
        } else {
            if( argv[i][0] != '-' )
                if(w)
                    try {
                        write_mp3s(argv[i], tag);
                    } catch(const out_of_range& x) {
                        printf("err: index out of range\n");
                    }
                else
                    printf("err: nothing to do with %s\n", argv[i]);
            else
                switch( toupper(argv[i][1]) ) {
#ifdef __ZF_SETID3V2
                case 'D':
                    if( frmID.assign(argv[i]+2) == "" )
                        tag.clear();
                    else
                        tag.rm(frmID);
                    frmID.erase();
                    w = true;
                    break;
#else
                case 'D': tag.clear(); w = true; break;
#endif
                case 'T': t = title;  break;
                case 'A': t = artist; break;
                case 'L': t = album;  break;
                case 'Y': t = year;   break;
                case 'C': t = cmnt;   break;
                case 'G': t = genre;  break;
                case 'N': t = track;  break;
#ifdef __ZF_SETID3V2
                case 'W': frmID.assign(argv[i]+2); break;
                case '2': tag.opt(aux++,true); break;
                case '1': tag.opt(true,aux++); break;
#endif
                default:
                    printf("err: unrecognized switch: -%c\n", argv[i][1]);
                    exit(1);
                }

        }
    }

    if(!w) help(argv[0]);
}

