#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>              // for some implementations of dirent.h
#endif
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
        return (void) printf("id3: could not read %s\n", path);

    bool m = false;                             // idle flag

    while( dirent* fn = readdir(dir) ) {
        varexp match(fspec, fn->d_name);
        strcpy(pname, fn->d_name);
        if( match && ++m && !tag.modify(path, match) )
            printf("id3: could not access %s!\n", fn->d_name);
    }
    closedir(dir);

    if(!m)
        printf("id3: no files matching %s\n", fspec);
}

/* ====================================================== */

const char shelp[] = "For help, type id3 -h\n";

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

int main_(int argc, char *argv[])
{
    ID3set t = ID3;
    bool   w = false;            // check against no-ops args
    bool   u = false;            // check against no-file args
#ifdef __ZF_SETID3V2
    bool aux = false;            // check for -1 & -2 commands
    string fieldID;
    smartID3v2 tag(true,false);  // default: write ID3v1, not v2
#else
    smartID3 tag;
#endif

    for(int i=1; i < argc; i++) {
#ifdef __ZF_SETID3V2
        if(!fieldID.empty()) {     // v2 - write raw frame
            tag.set(fieldID, argv[i]);
            fieldID.erase();
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
                    u=true, write_mp3s(argv[i], tag);
                else
                    u=true, printf("id3: nothing to do with %s\n", argv[i]);
            else
                switch( toupper(argv[i][1]) ) {
#ifdef __ZF_SETID3V2
                case 'D':
                    if( fieldID.assign(argv[i]+2) == "" )
                        tag.clear();
                    else
                        tag.rm(fieldID);
                    fieldID.erase();
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
                case 'W': fieldID.assign(argv[i]+2); break;
                case '2': tag.opt(aux++,true); break;
                case '1': tag.opt(true,aux++); break;
#endif
                case 'H': help(argv[0]);
                default:
                    printf("id3: unrecognized switch: -%c\n", argv[i][1]);
                    printf(shelp);
                    exit(1);
                }

        }
    }

    if(!u)
        printf("id3: missing file arguments\n");
    if(!u || !w)
        printf(shelp);
}


 // function-try blocks are not supported on some compilers,
 // so this little de-tour is necessary

int main(int argc, char *argv[])
{
    try {
        main_(argc, argv);
    } catch(const smartID3::failure& f) {
        printf("id3: %s\n", f.what());
    } catch(const out_of_range& x) {
        printf("id3: %s\n", x.what());
    } catch(const exception& exc) {
        printf("id3: unhandled exception: %s\n", exc.what());
    } catch(...) {
        printf("id3: unexpected unhandled exception\n");
    }
}
