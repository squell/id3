#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <cctype>
#include <clocale>
#include <stdexcept>
#include <string>
#include <memory>
#include "setgroup.h"
#include "setid3.h"
#include "setfname.h"
#include "setquery.h"
#ifndef LITE
#    include "setid3v2.h"
#    include "setlyr3.h"
#endif
#include "mass_tag.h"
#include "pattern.h"
#ifdef _WIN32
#    include <windows.h>
#endif

#define _version_ "0.79 (2015030)"

/*

  copyright (c) 2004-2006, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

namespace out = tag::write;
using namespace std;
using fileexp::mass_tag;
using tag::ID3field;
using tag::FIELD_MAX;

#if __cplusplus < 201103L
#define unique_ptr auto_ptr
#endif

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static const char* Name  = "id3";
static int         exitc = 0;

static void Copyright()
{
 //      |=======================64 chars wide==========================|
    printf(
        "%s " _version_ ", Copyright (C) 2003, 04, 05, 06, 15 Marc R. Schoolderman\n"
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
#ifndef LITE
        "usage: %s [-1 -2 -3] [OPTIONS] filespec ...\n"
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
#ifndef LITE
        "Only on last selected tag type:\n"
        " -s <size>\t"     "set tag size\n"
        " -E\t\t"          "only write if tag already exists\n"
        " -u\t\t"          "update all standard fields\n"
        " -rTYPE\t\t"      "erase all `TYPE' frames\n"
        " -wTYPE <data>\t" "write a `TYPE' frame\n"
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
#if defined(__DJGPP__) || defined(_WIN32)
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
    static bool enable;
private:
    static clock_t time;

    struct timer {
        timer() { time = clock(); }
       ~timer()
        {
            time = clock() - time;
            if(enable) {
                if(exitc!=0) fprintf(stderr, "Errors were encountered\n");
                if(exitc!=1) fprintf(stderr, "(%lu files in %.3fs) done\n", mass_tag::total(), double(time) / CLOCKS_PER_SEC);
            }
        }
    };
    friend struct timer;                                   // req by C++98

    virtual bool file(const char* name, const fileexp::record& f)
    {
        if(verbose::enable) {
            static timer initialize;
            if(counter==1 && name-f.path)
                 fprintf(stderr, "%.*s\n", int(name-f.path), f.path);
            fprintf(stderr, "\t%s\n", name);
        }
        if(! mass_tag::file(name, f) )
            eprintf("could not edit tag in %s\n", f.path);
        return 1;
    }
};

bool    verbose::enable;
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

    template<class T> struct box { T object; };
    template<class T> T& use(box<T>& x) { return x.object; }

    struct tag_info :
      out::query,
      box<out::ID3>,
#ifndef LITE
      box<out::ID3v2>,
      box<out::Lyrics3>,
      tag::reader,
#endif
      out::file
    {
#ifndef LITE
        tag::metadata* read(const char* fn) const
        {
            std::unique_ptr<tag::metadata> tag( box<out::ID3v2>::object.read(fn) );
            if(!tag.get() || !*tag)
                tag.reset( box<out::Lyrics3>::object.read(fn) );
            if(!tag.get() || !*tag)
                tag.reset( box<out::ID3>::object.read(fn) );
            return tag.release();
        }
#endif
    };

}

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

  // tag lister

struct listtag : fileexp::find {
    listtag(tag::reader& in) : m_reader(in)
    { }
    tag::reader& m_reader;

    static void content(const char* fmt, tag::metadata::value_string data)
    {
        if(data.good()) {
            string str = data.str();
            for(string::iterator p = str.begin(); p != str.end(); ++p) 
                if(std::iscntrl(*p)) *p = ' ';
            printf(fmt, str.c_str());
        }
    }

    virtual bool file(const char*, const fileexp::record& rec)
    {
        using namespace tag;
        std::unique_ptr<metadata> ptr( m_reader.read(rec.path) );
        if(ptr.get()) {
            const metadata& tag = *ptr;
            printf("File: %s\n", rec.path);
            if(tag) {
                content("Metadata: %s\n",tag[FIELD_MAX]);
                content("Title: %s\n",   tag[title]);
                content("Artist: %s\n",  tag[artist]);
                content("Album: %s\n",   tag[album]);
                content("Track: %s\n",   tag[track]);
                content("Year: %s\n",    tag[year]);
                content("Genre: %s\n",   tag[genre]);
                content("Comment: %s\n", tag[cmnt]);
            } else {
                content("Metadata: %s\n", "none found");
            }
            printf("\n");
        }
        return true;
    }
};

  // contains CLI interface loop

int main_(int argc, char *argv[])
{
    op::tag_info tag;

    ID3field field;
    const char*   val[FIELD_MAX] = { 0, };     // fields to alter

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
#ifndef LITE
                    source = &tag;             // use default tags
                    tag.with( use<out::ID3v2>(tag) );
                    tag.with( use<out::Lyrics3>(tag) );
#else
                    source = &use<out::ID3>(tag);
#endif
                    tag.with( use<out::ID3>(tag).create() );
                }
                for(int f = 0; f < FIELD_MAX; ++f)
                    if(val[f]) tag.set(ID3field(f), val[f]);
                if(copyfn && !tag.from(copyfn))
                    eprintf("note: could not read tags from %s\n", copyfn);
                if(state & clobr)
                    tag.rewrite();

                if(state & patrn) {
                    if(argv[i+1]) {
                        eprintf("-m %s: no file arguments are allowed\n", argv[i]);
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
                    if(chosen && tag.size() > 1)
                        eprintf("note: multiple selected tags ignored when reading\n");
                    tag.ignore(0, tag.size()); // don't perform no-ops
                case op::w | op::ren:
                case op::w:
                    break;
                default:
                    eprintf("cannot combine -q with any modifying operation\n");
                    shelp();
                case op::no_op:
                    listtag viewer(*source);
                    return process_(viewer, &argv[i], state & recur);
                }

                verbose tagger(*selected, *source);
                return process_(tagger, &argv[i], state & recur);
            } else {
                switch( *opt++ ) {             // argument is a switch
                case 'v': verbose::enable=1;  break;
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
#ifndef LITE
                    while(1) {
                case '1':
                        if(!source) source = &use<out::ID3>(tag);
                        chosen = &use<out::ID3>(tag);
                        break;
                case '2':                
                        if(!source) source = &use<out::ID3v2>(tag);
                        chosen = &use<out::ID3v2>(tag);
                        break;
                case '3':
                        if(!source) source = &use<out::Lyrics3>(tag);
                        chosen = &use<out::Lyrics3>(tag);
                        tag.with(use<out::ID3>(tag).create());
                        break;
                    }
                    tag.with(chosen->create());
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

                case 'E':
                    if(chosen) {
                        chosen->create(false);
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

#ifndef LITE
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
#if defined(__DJGPP__) || defined(_WIN32)
        char* end = strchr(prog, '\0');       // make "unixy" in appearance
        for(char* p = prog; p != end; ++p) *p = tolower(*p);
        if(end-prog > 4 && strcmp(end-4, ".exe") == 0) end[-4] = '\0';
#endif
        Name = prog;
    }
    try {
#  if defined(_WIN32)                         // set up locale
        char codepage[12];
        sprintf(codepage, ".%d", GetACP() & 0xFFFF);
        setlocale(LC_CTYPE, codepage);
        struct chcp {                         // fiddle with the console fonts
            int const cp_in, cp_out;
            chcp(int cp_new = GetACP()) 
            : cp_in(GetConsoleCP()), cp_out(GetConsoleOutputCP()) 
            { SetConsoleCP(cp_new), SetConsoleOutputCP(cp_new); }
            ~chcp()
            { SetConsoleCP(cp_in),  SetConsoleOutputCP(cp_out); }
        } lock;
#  else
        setlocale(LC_CTYPE, "");
#  endif
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
