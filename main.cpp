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

struct take_action {
    virtual bool operator()(const char*, varexp&) const = 0;
};

void for_files(const char* fspec, const take_action& action)
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
        if( match && ++m && !action(path, match) )
            printf("err: could not access %s!\n", fn->d_name);
    }
    closedir(dir);

    if(!m)
        printf("err: no files matching %s\n", fspec);
}

  /*
      Some history here: for_files() was called write_mp3s(),
      and it was hard-wired to call 'tag.modify()'. I added yet another
      layer of indirection to make this routine more versatile.
   */

/* ====================================================== */

struct write_mp3s : take_action {
    smartID3& tag;

    write_mp3s(smartID3& tag) : tag(tag) { }

    bool operator()(const char* path, varexp& vars) const
    { return tag.modify(path, vars); }
};

#include "id3_scm.h"

struct view_mp3s : take_action {
    mutable id3_scheme prn;

    bool operator()(const char* path, varexp&) const
    { return prn(path), true; }
};

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
#ifdef __ZF_SETID3V2
    smartID3v2 tag(true,false);  // default: write ID3v1, not v2
    string opt;                  // v2
#else
    smartID3 tag;
#endif

    for(int i=1; i < argc; i++) {
#ifdef __ZF_SETID3V2
        if(!opt.empty()) {       // v2
            tag.set(opt, argv[i]);
            opt.erase();
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
                    try{
                        for_files(argv[i], write_mp3s(tag));
                    //  for_files(argv[i], view_mp3s());
                    } catch(const out_of_range& x) {
                        printf("err: wildcard index out of range\n");
                    }
                else
                    printf("err: nothing to do with %s\n", argv[i]);
            else
                switch( toupper(argv[i][1]) ) {
#ifdef __ZF_SETID3V2
                case 'D':
                    if( opt.assign(argv[i]+2,4) == "" )
                        tag.clear();
                    else
                        tag.rm(opt);
                    opt.erase();
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
                case 'W': opt.assign(argv[i]+2,4); break;
                case '2': tag.v2(true).v1(argv[i][2]=='+'); break;
                case '1': tag.v1(true).v2(argv[i][2]=='+'); break;
#endif
                default:
                    printf("err: unrecognized switch: -%c\n", argv[i][1]);
                    exit(1);
                }

        }
    }
}

