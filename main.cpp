#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include "setgroup.h"
#include "setid3.h"
#include "setfname.h"
#include "setquery.h"
#ifndef NO_V2
#    include "setid3v2.h"
#endif
#include "mass_tag.h"
#include "pattern.h"

#define _version_ "0.77-2 (20060xx)"

/*

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

namespace out = tag::write;
using namespace std;
using fileexp::mass_tag;
using tag::ID3field;
using tag::FIELD_MAX;

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static const char* Name  = "id3";
static int         exitc = 0;

static void Copyright()
{
 //      |=======================64 chars wide==========================|
    printf(
        "%s " _version_ ", Copyright (C) 2003, 04, 05, 06 Marc R. Schoolderman\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; see the file named COPYING in the\n"
        "source distribution for details.\n",
        Name
    );
    exit(exitc=1);
}

static void Help()
{
    printf(
        "%s " _version_ "\n"
#ifndef NO_V2
        "usage: %s [-1 -2] [OPTIONS] filespec ...\n"
#else
        "usage: %s [OPTIONS] filespec ...\n"
#endif
        " -v\t\t"          "give verbose output\n"
        " -d\t\t"          "clear existing tag\n"
        " -t <title>\t"    "set tag fields\n"
        " -a <artist>\t"   "\n"
        " -l <album>\t"    "\t(i'th matched `*' wildcard  = %%1-%%9,%%0\n"
        " -n <tracknr>\t"  "\t path/file name/counters    = %%p %%f %%x %%X\n"
        " -y <year>\t"     "\t value of tag field in file = %%t %%a %%l %%n %%y %%g %%c)\n"
        " -g <genre>\t"    "\n"
        " -c <comment>\t"  "\n"
        " -D <filename\t"  "duplicate tags read from filename\n"
        " -f <template>\t" "rename files according to template\n"
        " -q <format>\t"   "print formatted string on standard output\n"
        " -m\t\t"          "match variables in filespec\n"
        " -R\t\t"          "search recursively\n"
        " -M\t\t"          "preserve modification time of files\n"
        " -V\t\t"          "print version info\n"
#ifndef NO_V2
        "Only on last selected tag:\n"
        " -s <size>\t"     "set tag size\n"
        " -u\t\t"          "update all standard fields\n"
        " -rXXXX\t\t"      "erase all XXXX frames\n"
        " -wXXXX <data>\t" "write a XXXX frame\n"
#endif
        "\nReport bugs to <squell@alumina.nl>.\n",
        Name,
        Name
    );
    exit(exitc=1);
}

static int shelp(bool quit = true)
{
    fprintf(stderr, "Try `%s -h' for more information.\n", Name);
    if(quit) exit  (exitc=1);
    else     return(exitc=1);
}

static void eprintf(const char* msg, ...)
{
    exitc = 2;
    va_list args;
    va_start(args, msg);
    fprintf  (stderr, "%s: ", Name);
    vfprintf (stderr, msg, args);
    va_end(args);
}

static long argtol(const char* arg)            // convert argument to long
{
    char* endp;
    long n = strtol(arg, &endp, 10);
    if(*endp != '\0') {
        eprintf("%s: invalid argument\n", arg);
        exit(exitc=1);
    }
    return n;
}

inline static char* argpath(char* arg)
{
#if defined(__DJGPP__) || defined(__WIN32__)
    for(char* p = arg; *p; ++p)                // convert backslashes
        if(*p == '\\') *p = '/';
#endif
    return arg;
}

/* ====================================================== */

class verbose : public mass_tag {
public:
    verbose(const tag::writer& write, const tag::reader& read)
    : mass_tag(write, read) { }
    static void enable(bool t = true) { verbose::show = t; }
private:
    static bool    show;
    static clock_t time;

    struct timer {
        timer() { time = clock(); }
       ~timer()
        {
            time = clock() - time;
            if(show) {
                if(exitc!=0) fprintf(stderr, "Errors were encountered\n");
                if(exitc!=1) fprintf(stderr, "(%lu files in %.3fs) done\n", mass_tag::total(), double(time) / CLOCKS_PER_SEC);
            }
        }
    };
    friend struct timer;                                   // req by C++98

    virtual bool file(const char* name, const fileexp::record& f)
    {
        if(verbose::show) {
            static timer initialize;
            if(counter==1 && name-f.path)
                 fprintf(stderr, "%.*s\n", name-f.path, f.path);
            fprintf(stderr, "\t%s\n", name);
        }
        if(! mass_tag::file(name, f) )
            eprintf("could not edit tag in %s\n", f.path);
        return 1;
    }
};

bool    verbose::show;
clock_t verbose::time;

/* ====================================================== */

namespace op {

    enum {                                     // state information bitset
        no_op =  0x00,                         
        recur =  0x01,                         // work recursively?
        w     =  0x02,                         // write  requested?
        ren   =  0x04,                         // rename requested?
        rd    =  0x08,                         // read   requested?
        clobr =  0x10,                         // clear  requested?
        patrn =  0x20                          // match  requested?
    };
    typedef int oper_t;

    struct tag_info :
      out::file,
      out::query
    {
        out::ID3      m_id3;
#ifndef NO_V2
        out::ID3v2    m_id3v2;
#endif
    };

}

struct null_op : fileexp::find {
    bool file(const char*, const fileexp::record& r)
    { return eprintf("nothing to do with %s\n", r.path), true; }
};

/* ====================================================== */

  // performs the actual operations

int process_(fileexp::find& work, char* files[], bool recur)
{
    do {
        if(! work.glob(argpath(*files), recur) )
            eprintf("no files matching %s\n", *files);
    } while(*++files);
    return exitc;
}

  // contains CLI interface loop

int main_(int argc, char *argv[])
{
    op::tag_info tag;

    ID3field field;
    const char*   val[FIELD_MAX] = { };        // fields to alter

    tag::reader*  source  = 0;                 // pointer to first enabled
    tag::handler* chosen  = 0;                 // pointer to last enabled
    const char*   copyfn  = 0;                 // alternate from-file

    using namespace op;
    char none[] = "";

    enum parm_t {                              // parameter modes
        no_value, force_fn,
        std_field, custom_field, suggest_size,
        set_rename, set_query, set_copyfrom
    };

    parm_t cmd   = no_value;
    oper_t state = no_op;
    char*  opt   = none;                       // used for command stacking

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command argument
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-') opt = argv[i]+1;
            if(*opt == '\0') {
        case force_fn:                         // argument is filespec
                if(!chosen) {
                    source = &tag.m_id3;       // use default tags
                    tag.with( tag.m_id3.create() );
                } else {                       // modify source tag as last
                    swap(tag[0], tag[tag.size()-1]);     
                }
                for(int f = 0; f < FIELD_MAX; ++f)
                    if(val[f]) tag.set(ID3field(f), val[f]);
                if(copyfn && !tag.from(copyfn))
                    eprintf("note: could not read tags from %s\n", copyfn);
                if(state & clobr)
                    tag.rewrite();

                if(state & patrn) {
                    if(argv[i+1]) {
                        eprintf("-m: unexpected extraneous file arguments given\n", argv[i]);
                        shelp();
                    }
                    pattern spec(tag, argpath(argv[i]));
                    if(spec.vars() > 0) state |= w;
                    strcpy(argv[i], spec.c_str());
                }

                tag::writer* selected = &(out::file&) tag;

                switch(state & (w|rd|ren)) {
                case op::rd:
                    selected = &(out::query&) tag;
                case op::ren:
                    if(tag.size() > 1)
                        eprintf("note: multiple selected tags ignored when reading\n");
                    tag.ignore(0, tag.size()); // don't perform no-ops
                case op::w | op::ren:
                case op::w:
                    break;
                default:
                    eprintf("cannot combine -q with any modifying operation\n");
                    shelp();
                case op::no_op:
                    null_op dummy;
                    return process_(dummy, &argv[i], state & recur);
                }

                verbose tagger(*selected, *source);
                return process_(tagger, &argv[i], state & recur);
            } else {
                switch( *opt++ ) {             // argument is a switch
                case 'v': verbose::enable(); break;
                case 'M': tag.touch(false);   break;
                case 'f': cmd = set_rename;   break;
                case 'q': cmd = set_query;    break;

                case 'm': if(!(state & recur)) {
                              state |= patrn; break;
                          } else if(false)     // skip next statement
                case 'R': if(!(state & patrn)) {
                              state |= recur; break;
                          }
                    eprintf("cannot use -R and -m at the same time\n");
                    shelp();

                case 'D': if(!(state&clobr)) {
                              cmd = set_copyfrom; break;
                          }
                case 'd': if(!(state&clobr)) {
                              state |= (w|clobr); break;
                          }
                    eprintf("cannot use either -d or -D more than once\n");
                    shelp();

                default:
                    field = mass_tag::field(opt[-1]);
                    if(field == FIELD_MAX) {
                        eprintf("-%c: unrecognized switch\n", opt[-1]);
                        shelp();
                    }
                    cmd = std_field; break;
#ifndef NO_V2
                case '1':
                    tag.with( *(chosen = &tag.m_id3.create()) );
                    if(!source) source = &tag.m_id3;
                    break;
                case '2':
                    tag.with( *(chosen = &tag.m_id3v2.create()) );
                    if(!source) source = &tag.m_id3v2;
                    break;

                case 's':                      // tag specific switches
                    if(chosen) {
                        if(*opt == '\0')
                            cmd = suggest_size;
                        else {
                            long n = argtol(opt);
                            chosen->reserve(n);
                            state |= w;
                        }
                        opt = none;
                        break;
                    }
                case 'w':
                    if(chosen) {
                        cmd = custom_field;
                        break;
                    }
                case 'r':
                    if(chosen) {
                        if(! chosen->rm(opt) ) {
                            eprintf("selected tag does not have `%s' frames\n", opt);
                            shelp();
                        }
                        state |= w;
                        opt = none;
                        break;
                    }
#endif
                case 'u':
                    if(chosen) {
                        for(int i = 0; i < FIELD_MAX; ++i)
                            chosen->set(ID3field(i), mass_tag::var(i));
                        state |= w;
                        break;
                    }

                    eprintf("specify tag format before -%c\n", opt[-1]);
                    shelp();

                case 'h': Help();
                case 'V': Copyright();
                case '-':
                    if(opt == argv[i]+2 && *opt == '\0') {
                       cmd = force_fn;
                       break;
                    }
                }
            }
            continue;

        case std_field:                        // write a standard field
            val[field] = argv[i];
            break;

        case set_copyfrom:                     // specify source tag
            copyfn = argv[i];
            state |= clobr;
            break;

#ifndef NO_V2
        case custom_field:                     // v2 - write a custom field
            if(! chosen->set(opt, argv[i]) ) {
                eprintf("selected tag does not have `%s' frames\n", opt);
                shelp();
            }
            opt = none;
            break;

        case suggest_size: {                   // v2 - suggest size
                long n = argtol(argv[i]);
                chosen->reserve(n);
            }
            break;
#endif

        case set_rename:                       // specify rename format
            if(strrchr(argv[i],'/')) {
                eprintf("will not rename across directories\n");
                shelp();
            } else if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
                shelp();
            } else
                tag.rename( argpath(argv[i]) );
            state |= ren;
            cmd = no_value;
            continue;

        case set_query:                        // specify echo format
            tag.print( argv[i] );
            state |= rd;
            cmd = no_value;
            continue;
        }

        state |= w;                            // set operation done flag
        cmd = no_value;
    }

    eprintf("missing file arguments\n");
    if(state == no_op) shelp();

    return exitc;
}

 // function-try blocks are not supported on some compilers (borland),
 // so this little de-tour is necessary

int main(int argc, char *argv[])
{
    if(char* prog = argv[0]) {                // set up program name
        if(char* p = strrchr(argpath(prog), '/')) prog = p+1;
#if defined(__DJGPP__) || defined(__WIN32__)
        char* end = strchr(prog, '\0');       // make "unixy" in appearance
        for(char* p = prog; p != end; ++p) *p = tolower(*p);
        if(end-prog > 4 && strcmp(end-4, ".exe") == 0) end[-4] = '\0';
#endif
        Name = prog;
    }
    try {
        return main_(argc, argv);
    } catch(const tag::failure& f) {
        eprintf("%s (tagging aborted)\n", f.what());
    } catch(const out_of_range& x) {
        eprintf("%s\n", x.what());
        return shelp(0);
    } catch(const exception& exc) {
        eprintf("unhandled exception: %s\n", exc.what());
    } catch(...) {
        eprintf("unexpected unhandled exception\n");
    }
    return 3;
}

void deprecated(const char* msg)
{
    eprintf("%s\n", msg);
}

