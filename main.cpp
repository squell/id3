#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include "ffindexp.h"
#include "sedit.h"

#include "set_base.h"
#include "setid3.h"
#include "setfname.h"
#ifndef NO_V2
#  include "setid3v2.h"
#endif

#define _version_ "0.74-beta (2004176)"

/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static int exitc = 0;

 // file handle to dump errors to

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
    {
        time = clock() - time;
        if(show) {
            if(exitc!=0) printf("Errors were encountered\n");
            if(exitc!=1) printf("(%.3fs) done\n", double(time) / CLOCKS_PER_SEC);
        }
    }

    void reportf(const char* s)                     // reporting a filename
    {
        if(show) {
            const char* sep = strrchr(s, '/');
            printf("\t%s\n", sep?sep+1:s);
        }
    }
    void reportd(const char* s)                     // reporting a dir
    {   if(show && *s) printf("%s\n", s);   }
} verbose;

/* ====================================================== */

namespace {
    using namespace set_tag;

   // this template class:
   // - boxes a handler to make it safe for (multiple) inheritance,
   // - defaults the handler to 'disabled',
   // - delegates it into a most-derived-shared combined_tag object

    template<class T> struct uses : virtual combined_tag {
        uses(bool on = false) : object(on)
        { combined_tag::delegate(object); }

        T object;
    };

    // next function acts like a cast operator on the above

    template<class T> inline T& with(T& obj)                   { return obj; }
    template<class T> inline T& with(uses<T>& box)             { return box.object; }
    template<class T> inline const T& with(const T& obj)       { return obj; }
    template<class T> inline const T& with(const uses<T>& box) { return box.object; }
}

#ifdef __ZF_SETID3V2

#  ifdef NO_FN
struct metadata : uses<ID3v2>, uses<ID3> { };
#  else
struct metadata : uses<ID3v2>, uses<ID3>, uses<filename> { };
#  endif

#else

typedef ID3 metadata;

#endif

/* ====================================================== */

class mass_tag : filefindexp, public metadata {
    virtual void process();
    virtual void entered();

    bool edir;
public:
    mass_tag() : edir(false) { }
    void operator()(const char* spec);

    template<class Tag> single_tag& enable()
    { return with<Tag>(*this).active(true); }
};

void mass_tag::entered()
{
    verbose.reportd(path);
    edir = true;
}

void mass_tag::process()
{
    verbose.reportf(path);
    if(! modify(path, var) )
        return (void) fprintf(err(), "id3: could not edit tag in %s\n", path);
}

void mass_tag::operator()(const char* spec)
{
    if(! filefindexp::operator()(spec) )
        fprintf(err(), "id3: no %s matching %s\n",
                edir? "files" : "directories", spec);
}

/* ====================================================== */

namespace {

void help(const char* argv0)
{
    printf(
#ifdef __ZF_SETID3V2
        "id3 " _version_ "\n"
        "usage: %s [-1 -2] [OPTIONS] filespec ...\n"
#else
        "usage: %s [OPTIONS] filespec ...\n"
#endif
        " -d\t\t"          "clear existing tag\n"
        " -t <title>\t"    "set fields\n"
        " -a <artist>\t"   "\n"
        " -l <album>\t"    "\n"
        " -n <tracknr>\t"  "\n"
        " -y <year>\t"     "\n"
        " -g <genre>\t"    "\n"
        " -c <comment>\t"  "\n"
        " -f <filename>\t" "set filename\n"
        " -v\t\t"          "give verbose output\n"
        " -V\t\t"          "print version info\n"
#ifdef __ZF_SETID3V2
        "Only when -2:\n"
        " -rXXXX\t\terase all XXXX frames\n"
        " -wXXXX <data>\twrite a XXXX frame\n"
        " -s <size>\tforce size of final tag\n"
#endif
        "\nAny occurences of the form \"%%i\" in an ID3 field value will be substituted by\n"
        "the portion of the actual filename matched by the i'th \"*\" wildcard, where i\n"
        "is a digit in the range [1..9,0].\n\n"
        "Report bugs to <squell@alumina.nl>.\n",
        argv0
    );
    exit(exitc=1);
}

void Copyright()
{
 //      |=======================64 chars wide==========================|
    printf(
        "id3 " _version_ ", Copyright (C) 2003, 04 Marc R. Schoolderman\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; see the file named COPYING in the\n"
        "source distribution for details.\n"
    );
    exit(exitc=1);
}

void shelp()
{
    fprintf(err(), "Try `id3 -h' for more information.\n");
    exit(exitc=1);
}

long argtol(const char* arg)                   // convert argument to long
{
    char* endp;
    long n = strtol(arg, &endp, 0);
    if(*endp != '\0') {
        fprintf(err(), "id3: invalid argument `%s'\n", arg);
        exit(exitc=1);
    }
    return n;
}

#if 0
inline void argpath(char* arg) { }             // dummy
#else
void argpath(char* arg)                        // convert backslashes
{
    for(char* p = arg; *p; ++p)
        if(*p == '\\') *p = '/';
}
#endif

}

/* ====================================================== */

using set_tag::ID3field;

int main_(int argc, char *argv[])
{
    mass_tag tag;

    enum parm_t {                              // parameter modes
        no_value, force_fn,
        stdfield, customfield, suggest_size,
        set_rename
    } cmd = no_value;

    ID3field field;
    string fieldID;                            // free form field selector

    set_tag::handler* chosen = 0;              // pointer to last enabled

    char* opt  = "";                           // used for command stacking
    bool  scan = true;                         // check for no-file args
    bool  w    = false;                        // check against no-ops args

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command parameter
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-' && scan) opt = argv[i]+1;
        case force_fn:
            if(*opt == '\0') {
                argpath(argv[i]);
                scan = false;
                if(!chosen)                    // default to ID3
                    tag.enable<ID3>();
                if(w)                          // no-op check
                    tag( argv[i] );
                else
                    fprintf(err(), "id3: nothing to do with %s\n", argv[i]);
            } else {
                switch( *opt++ ) {             // param is an option
                case 'v': verbose.on(); break;
                case 'd': tag.clear(); w = true; break;
                case 't': field = set_tag::title;  cmd = stdfield; break;
                case 'a': field = set_tag::artist; cmd = stdfield; break;
                case 'l': field = set_tag::album;  cmd = stdfield; break;
                case 'y': field = set_tag::year;   cmd = stdfield; break;
                case 'c': field = set_tag::cmnt;   cmd = stdfield; break;
                case 'g': field = set_tag::genre;  cmd = stdfield; break;
                case 'n': field = set_tag::track;  cmd = stdfield; break;
                case 'f': cmd = set_rename; break;
#ifdef __ZF_SETID3V2
                case '1':
                    chosen = &tag.enable<ID3>();
                    break;
                case '2':
                    chosen = &tag.enable<ID3v2>();
                    break;

                case 's':                      // tag specific options
                    if(chosen) {
                        cmd = suggest_size; break;
                    }
                case 'w':
                    if(chosen) {
                        fieldID.assign(opt); cmd = customfield;
                        opt = "";
                        break;
                    }
                case 'r':
                    if(chosen) {
                        chosen->rm(opt); w = true;
                        opt = "";
                        break;
                    }
                    fprintf(err(), "id3: specify tag before -%c\n", opt[-1]);
                    shelp();
#endif
                case 'h': help(argv[0]);
                case 'V': Copyright();
                case '-':
                    if(opt == argv[i]+2 && *opt == '\0') {
                       cmd = force_fn;
                       break;
                    }
                default:
                    fprintf(err(), "id3: unrecognized switch: -%c\n", opt[-1]);
                    shelp();
                }
            }
            continue;

        case stdfield:                         // write a standard field
            tag.set(field, argv[i]);
            break;

        case set_rename:
            argpath(argv[i]);
            if(! with<filename>(tag).rename(argv[i]) ) {
                fprintf(err(), "id3: will not rename across directories\n");
                shelp();
            }
            if(!chosen)
                chosen = &with<filename>(tag);
            break;

#ifdef __ZF_SETID3V2
        case suggest_size: {                   // v2 - suggest size
                long l = argtol(argv[i]);
                chosen->reserve(l);
            }
            break;

        case customfield:                      // v2 - write a custom field
            if(! chosen->set(fieldID, argv[i]) ) {
                fprintf(err(), "id3: cannot write frame `%s'\n", fieldID.c_str());
                shelp();
            }
            break;
#endif
        };
        cmd = no_value;
        w = true;                              // set operation done flag
    }

    if(scan)
        fprintf(err(), "id3: missing file arguments\n");
    if(scan || !w)
        shelp();

    return exitc;
}

 // function-try blocks are not supported on some compilers (borland),
 // so this little de-tour is necessary

int main(int argc, char *argv[])
{
    argpath(argv[0]);
    try {
        return main_(argc, argv);
    } catch(const set_tag::failure& f) {
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

