#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>              // for some implementations of dirent.h
#endif
#include <dirent.h>

#if defined(__WIN32__)
#  define _POSIX_ 1                // borland c++ needs this
#endif
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <ctime>

#include <vector>
#include <string>

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

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static int exitc = 0;

static inline FILE* err()
{
    exitc = 2;
    return stderr;
}

 // all verbose mode functionality goes here

struct verbose_t {
    bool    show;
    clock_t time;

    void on()
    {   show = true;   }

    verbose_t() : show(false), time(clock()) { }

    ~verbose_t()
    {   time = clock() - time;
        if(show) {
            if(exitc!=0) printf("Errors were encountered\n");
            if(exitc!=1) printf("(%.3fs) done\n", double(time) / CLOCKS_PER_SEC);
        }                                                                   }

    void report(const char* s, bool m)            // reporting a filename
    {   if(show && m) printf("%s\n", s);   }
} verbose;

/* ====================================================== */

 // mini class for reading a directory as a vector

struct dirvector : vector<string> {
    dirvector(const char* path);
    operator bool()
    { return success; }

    bool success;
};

dirvector::dirvector(const char* path)
{
    if( DIR* dir = opendir(path) ) {
        while( dirent* fn = readdir(dir) ) {
            push_back(fn->d_name);
        }
        closedir(dir);
        success = true;
    } else {
        success = false;
    }
}

/* ====================================================== */

void write_mp3s(const char* fspec, smartID3& tag)
{
    char path[PATH_MAX];
    strncpy(path, fspec, sizeof path);          // copy constant
    path[sizeof path-1] = 0;                    // duct tape

    char* pname = strrchr(path, '/');           // pos to append filename to
    if(pname) {
        *(++pname) = 0;                         // seperate path & fspec
        fspec += (pname-path);                  // increase expression ptr
    } else {
        pname = strcpy(path, "./");
    }

    dirvector dir(path);
    if(!dir)
        return (void) fprintf(err(), "id3: could not read %s\n", path);

    bool m = false;                             // idle flag

    for(dirvector::iterator fn = dir.begin(); fn != dir.end(); ++fn) {
        strcpy(pname, fn->c_str());
        varexp match(fspec, pname);
        verbose.report(path, match);
        if( match && ++m && !tag.modify(path, match) )
            fprintf(err(), "id3: could not access %s!\n", pname);
    }

    if(!m)
        fprintf(err(), "id3: no files matching %s\n", fspec);
}

/* ====================================================== */

void shelp()
{
    fprintf(err(), "Try `id3 -h' for more information.\n");
    exit(exitc=1);
}

void help(const char* argv0)
{
    printf(
        "id3 0.72 (20040xx)\n"
#ifdef __ZF_SETID3V2
        "usage: %s [-1 -2] [OPTIONS] filespec ...\n\n"
#else
        "usage: %s [OPTIONS] filespec ...\n\n"
#endif
        " -v\tgive verbose output\n"
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
        " -rXXXX\terase all XXXX frames\n"
        " -wXXXX <data>\n\tdirectwrite an XXXX frame\n"
#endif
        "\nAny occurences of the form \"%%i\" in an ID3 field value will be substituted by\n"
        "the portion of the actual filename matched by the i'th \"*\" wildcard, where i\n"
        "is a digit in the range [1..9,0].\n",
        argv0
    );
    exit(exitc=1);
}

/* ====================================================== */

int main_(int argc, char *argv[])
{
    enum parm_t {
        no_value, stdfield,  customfield,
    } cmd = no_value;

    bool   w = false;                          // check against no-ops args
    bool   u = false;                          // check against no-file args

    ID3set field;
#ifdef __ZF_SETID3V2
    string fieldID;

    bool aux = false;                          // check for -1 & -2 commands
    smartID3v2 tag(true,false);                // def: write ID3v1, not v2
#else
    smartID3 tag;
#endif
    char* opt = "";                            // used for command stacking

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command parameter
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-') opt = argv[i]+1;
            if(*opt == '\0')
                if(w)
                    u=true, write_mp3s(argv[i], tag);
                else
                    u=true, fprintf(err(), "id3: nothing to do with %s\n", argv[i]);
            else                       
                switch( toupper(*opt++) ) {    // param is an option
                case 'V': verbose.on(); break;
                case 'D': tag.clear(); w = true; break;
                case 'T': field = title;  cmd = stdfield; break;
                case 'A': field = artist; cmd = stdfield; break;
                case 'L': field = album;  cmd = stdfield; break;
                case 'Y': field = year;   cmd = stdfield; break;
                case 'C': field = cmnt;   cmd = stdfield; break;
                case 'G': field = genre;  cmd = stdfield; break;
                case 'N': field = track;  cmd = stdfield; break;
#ifdef __ZF_SETID3V2
                case 'W': 
                    fieldID.assign(opt); opt = "";
                    cmd = customfield; break;
                case 'R':
                    tag.rm(opt); opt = "";
                    w = true; break;
                case '2': tag.opt(aux++,true); break;
                case '1': tag.opt(true,aux++); break;
#endif
                case 'H': help(argv[0]);
                default:
                    fprintf(err(), "id3: unrecognized switch: -%c\n", opt[-1]);
                    shelp();
                }
            continue;

        case stdfield:                         // write a standard field
            tag.set(field, argv[i]);
            break;

#ifdef __ZF_SETID3V2
        case customfield:                      // v2 - write a custom field
            tag.set(fieldID, argv[i]);
            fieldID.erase();
            break;
#endif
        };
        cmd = no_value;
        w = true;
    }

    if(!u)
        fprintf(err(), "id3: missing file arguments\n");
    if(!u || !w)
        shelp();

    return exitc;
}

 // function-try blocks are not supported on some compilers (borland),
 // so this little de-tour is necessary

int main(int argc, char *argv[])
{
    try {
        return main_(argc, argv);
    } catch(const smartID3::failure& f) {
        fprintf(err(), "id3: %s\n", f.what());
    } catch(const out_of_range& x) {
        fprintf(err(), "id3: %s\n", x.what());
    } catch(const exception& exc) {
        fprintf(err(), "id3: unhandled exception: %s\n", exc.what());
    } catch(...) {
        fprintf(err(), "id3: unexpected unhandled exception\n");
    }
    return 3;
}

