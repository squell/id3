#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <climits>
#include <ctime>
#include <stdexcept>
#include <string>
#include "ffindexp.h"
#include "verbose.h"

#include "set_base.h"
#include "setid3.h"
#include "setfname.h"
#include "setecho.h"
#ifndef NO_V2
#    include "setid3v2.h"
#endif

#define _version_ "0.75 (2005034)"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static char*     name  = "id3";
static verbose_t verbose;
static int&      exitc = verbose.exitc;

static void eprintf(const char* msg, ...)
{
    exitc = 2;
    va_list args;
    va_start(args, msg);
    fprintf  (stderr, "%s: ", name);
    vfprintf (stderr, msg, args);
    va_end(args);
}

/* ====================================================== */

namespace {

#ifndef NO_V2
    using set_tag::ID3v2;
#endif
    using set_tag::ID3;
    using set_tag::filename;

   // this template class:
   // - boxes a handler to make it safe for (multiple) inheritance,
   // - defaults the handler to 'disabled',
   // - delegates it into a most-derived-shared combined object

    template<class T> struct uses : virtual set_tag::combined {
        uses(bool on = false) : object(on)
        { delegate(object); }

        T object;
    };

    // next function acts like a cast operator on the above

    template<class T> inline T& with(T& obj)                   { return obj; }
    template<class T> inline T& with(uses<T>& box)             { return box.object; }
    template<class T> inline const T& with(const T& obj)       { return obj; }
    template<class T> inline const T& with(const uses<T>& box) { return box.object; }
}

struct metadata :
#ifndef NO_V2
   uses<ID3v2>,
#endif
   uses<ID3>,
   uses<filename>
{
    template<class Tag> Tag* enable()
    { return (Tag*) &with<Tag>(*this).active(true); }
};

/* ====================================================== */

 // range checked vector

struct safe {
    safe(const vector<string>& v) : vec(v) { }
    inline const string& operator[](size_t i) const
    {
        if(i >= vec.size())
            throw out_of_range("variable index out of range");
        return vec[i];
    }
private:
    const vector<string>& vec;
};

/* ====================================================== */

  // 'lazy evaluation pointer' - only acquires resource when necessary

struct lazyreader {
    const char*              const filename;
    const set_tag::provider* const tag;

    lazyreader(const set_tag::provider* ctor, const char* fn)
    : filename(fn), tag(ctor), data(0) { }
   ~lazyreader()
    { delete data; }

    const set_tag::reader& operator*() const;
private:
    mutable const set_tag::reader* data;
};

const set_tag::reader& lazyreader::operator*() const
{
    return *(data? data : data = tag->read(filename));
}

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

class substvars {
    static unsigned  counter;
    const lazyreader data;
public:
    substvars(const set_tag::provider& ctor, const char* fn)
    : data(&ctor, fn) { }
    cvtstring operator[](char field) const;
};

cvtstring substvars::operator[](char field) const
{
    switch( field ) {
    case 't': return (*data)[set_tag::title];
    case 'a': return (*data)[set_tag::artist];
    case 'l': return (*data)[set_tag::album];
    case 'y': return (*data)[set_tag::year];
    case 'c': return (*data)[set_tag::cmnt];
    case 'n': return (*data)[set_tag::track];
    case 'g': return (*data)[set_tag::genre];
    case 'f': if(const char* p = strrchr(data.filename,'/'))
                  return cvtstring::local(p+1);
              else
                  return cvtstring::local(data.filename);
    case 'x': {
            counter = (counter+1) & 0xFFFF;
            char buf[11];
            sprintf(buf, "%u", counter);
            return cvtstring::latin1(buf);
        }
    };
    static char error[] = "unknown variable %_";
    error[sizeof error-2] = field;
    throw set_tag::failure(error);
}

unsigned substvars::counter = 0;

/* ====================================================== */

 // adaptation of filefindexp with verbose output

class mass_tag : filefindexp {
    const set_tag::handler*  tag;
    const set_tag::provider* info;
    virtual void entered();
    virtual void process();
    bool edir;
public:
    mass_tag() : edir(false) { }
    void operator()
      ( const set_tag::handler&, const char*, const set_tag::provider& );
};

void mass_tag::operator()(const set_tag::handler& h, const char* spec, const set_tag::provider& p)
{
    tag  = &h;
    info = &p;
    if(! filefindexp::operator()(spec) )
        eprintf("no %s matching %s\n", edir? "files" : "directories", spec);
}

void mass_tag::entered()
{
    verbose.reportd(path);
    edir = true;
}

void mass_tag::process()
{
    verbose.reportf(path);
    if(! tag->modify(path, safe(var), substvars(*info,path)) )
        return (void) eprintf("could not edit tag in %s\n", path);
}

/* ====================================================== */

static void help()
{
    printf(
#ifndef NO_V2
        "%s " _version_ "\n"
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
        " -q <string>\t"   "print string on stdout\n"
        " -v\t\t"          "give verbose output\n"
        " -V\t\t"          "print version info\n"
#ifndef NO_V2
        "Only on last selected tag:\n"
        " -rXXXX\t\terase all XXXX frames\n"
        " -wXXXX <data>\twrite a XXXX frame\n"
        " -s <size>\tforce size of final tag\n"
        " -!\t\t"          "force rewrite of tag\n"
#endif
        "\nAny occurences of the form \"%%i\" in an ID3 field value will be substituted by\n"
        "the portion of the actual filename matched by the i'th \"*\" wildcard, where i\n"
        "is a digit in the range [1..9,0].\n\n"
        "Report bugs to <squell@alumina.nl>.\n",
        name,
        name
    );
    exit(exitc=1);
}

static void Copyright()
{
 //      |=======================64 chars wide==========================|
    printf(
        "%s " _version_ ", Copyright (C) 2003, 04, 05 Marc R. Schoolderman\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; see the file named COPYING in the\n"
        "source distribution for details.\n",
        name
    );
    exit(exitc=1);
}

static void shelp()
{
    fprintf(stderr, "Try `%s -h' for more information.\n", name);
    exit(exitc=1);
}

static long argtol(const char* arg)            // convert argument to long
{
    char* endp;
    long n = strtol(arg, &endp, 0);
    if(*endp != '\0') {
        eprintf("invalid argument `%s'\n", arg);
        exit(exitc=1);
    }
    return n;
}

#if defined(__DJGPP__) || defined(__WIN32__)
static void argpath(char* arg)                 // convert backslashes
{
    for(char* p = arg; *p; ++p)
        if(*p == '\\') *p = '/';
}
#else
static inline void argpath(char* arg) { }      // dummy
#endif

/* ====================================================== */

void defaults(metadata& tag, set_tag::handler*& target,
                             set_tag::provider*& source)
{
    typedef set_tag::ID3 Default;

    if(!target) target = &with<Default>(tag).active(true);
    if(!source) source = &with<Default>(tag);
}

using set_tag::ID3field;

int main_(int argc, char *argv[])
{
    set_tag::echo display;
    mass_tag apply;
    metadata tag;

    enum parm_t {                              // parameter modes
        no_value, force_fn,
        stdfield, customfield, suggest_size,
        set_rename, set_query
    } cmd = no_value;

    ID3field field;
    string fieldID;                            // free form field selector

    set_tag::provider* source = 0;             // pointer to first enabled
    set_tag::handler*  chosen = 0;             // pointer to last enabled

    char* opt  = "";                           // used for command stacking
    bool  scan = true;                         // check for no-file args
    bool  w    = false;                        // check against no-ops args
    bool  ren  = false;                        // check for file renaming
    bool  ro   = false;                        // check for read-only ops

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command parameter
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-' && scan) opt = argv[i]+1;
        case force_fn:
            if(*opt == '\0') {
                defaults(tag, chosen, source);
                argpath(argv[i]);
                scan = false;
                if(w && !ro)                   // no-op check
                    apply(tag, argv[i], *source);
                else if(ren && !ro)
                    apply(with<filename>(tag), argv[i], *source);
                else if(ro)                    // reading?
                    if(!w && !ren) {
                        apply(display, argv[i], *source);
                    } else {
                        eprintf("incompatible operation requested\n");
                        shelp();
                    }
                else
                    eprintf("nothing to do with %s\n", argv[i]);
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
                case 'q': cmd = set_query;  break;
#ifndef NO_V2
                case '1':
                    chosen = tag.enable<ID3>();
                    if(!source) source = &with<ID3>(tag);
                    break;
                case '2':
                    chosen = tag.enable<ID3v2>();
                    if(!source) source = &with<ID3v2>(tag);
                    break;

  // tag specific options

                case 's':
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
#endif
                case '!':
                    if(chosen) {
                        chosen->set(set_tag::title,  "%t");
                        chosen->set(set_tag::artist, "%a");
                        chosen->set(set_tag::album,  "%l");
                        chosen->set(set_tag::year,   "%y");
                        chosen->set(set_tag::cmnt,   "%c");
                        chosen->set(set_tag::genre,  "%g");
                        chosen->set(set_tag::track,  "%n");
                        w = true;
                        break;
                    }

                    eprintf("specify tag format before -%c\n", opt[-1]);
                    shelp();

                case 'h': help();
                case 'V': Copyright();
                case '-':
                    if(opt == argv[i]+2 && *opt == '\0') {
                       cmd = force_fn;
                       break;
                    }
                default:
                    eprintf("unrecognized switch: -%c\n", opt[-1]);
                    shelp();
                }
            }
            continue;

        case stdfield:                         // write a standard field
            tag.set(field, argv[i]);
            break;

#ifndef NO_V2
        case suggest_size: {                   // v2 - suggest size
                long l = argtol(argv[i]);
                chosen->reserve(l);
            }
            break;

        case customfield:                      // v2 - write a custom field
            if(! chosen->set(fieldID, argv[i]) ) {
                eprintf("cannot write `%s' frames\n", fieldID.c_str());
                shelp();
            }
            break;
#endif
        case set_rename:
            if(strrchr(argv[i],'/')) {
                eprintf("will not rename across directories\n");
            } else if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
            } else {
                argpath(argv[i]);
                tag.enable<filename>()->rename(argv[i]);
                cmd = no_value;
                ren = true;
                continue;
            }
            shelp();

        case set_query:
            if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
                shelp();
            } else
                display.format(argv[i]);
            cmd = no_value;
            ro = true;
            continue;
        };
        cmd = no_value;
        w = true;                              // set operation done flag
    }

    if(scan)
        eprintf("missing file arguments\n");
    if(scan || !(w|ren|ro))
        shelp();

    return exitc;
}

 // function-try blocks are not supported on some compilers (borland),
 // so this little de-tour is necessary

int main(int argc, char *argv[])
{
    argpath(name=argv[0]);                    // set up program name
    if(char* p = strrchr(argv[0], '/')) name = p+1;

#if defined(__DJGPP__) || defined(__WIN32__)
    char* end = strchr(name, '\0');           // make "unixy" in appearance
    for(char* p = name; p != end; ++p) *p = tolower(*p);
    if(end-name > 4 && strcmp(end-4, ".exe") == 0) end[-4] = '\0';
#endif

    try {
        return main_(argc, argv);
    } catch(const set_tag::failure& f) {
        eprintf("%s\n", f.what());
    } catch(const out_of_range& x) {
        eprintf("%s\n", x.what());
    } catch(const exception& exc) {
        eprintf("unhandled exception: %s\n", exc.what());
    } catch(...) {
        eprintf("unexpected unhandled exception\n");
    }
    return 3;
}

