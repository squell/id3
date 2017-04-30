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
#include "setid3v2.h"
#include "setlyr3.h"
#include "mass_tag.h"
#include "pattern.h"
#include "dumptag.h"
#include "id3v1.h"
#ifdef _WIN32
#    include <windows.h>
#endif

#define _version_ "0.80 (2016005)"

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

static const char* const Options[] = {
    "v", "-verbose",
    "d", "-delete",
    "t", "-title",
    "a", "-artist",
    "l", "-album",
    "n", "-track",
    "y", "-year",
    "g", "-genre",
    "c", "-comment",
    "D", "-duplicate",
    "f", "-rename",
    "q", "-query",
    "m", "-match",
    "R", "-recursive",
    "X", "-no-glob",
    "M", "-keep-time",
    "L", "-list-genres",
    "V", "-version",
    "s", "-size",
    "E", "-if-exists",
    "u", "-update",
    "r", "-remove=",
    "w", "-frame=",
    "1", "-id3v1",
    "2", "-id3v2",
    "3", "-lyrics3",
    "?", "-help"
};

static void Help(bool long_opt=false)
{
    const char*const* const flags = Options+long_opt;
    printf(
        "%s " _version_ "\n"
        "usage: %s [-1 -2 -3] [OPTIONS] filespec ...\n"
        " -%s\t\t"          "give verbose output\n"
        " -%s\t\t"          "clear existing tag\n"
        " -%s <title>\t"    "set tag fields\n"
        " -%s <artist>\t"   "\n"
        " -%s <album>\t"    "   (i'th matched `*' wildcard  = %%1-%%9,%%0\n"
        " -%s <tracknr>\t"  "    path/file name/counters    = %%p %%f %%x %%X\n"
        " -%s <year>  \t"   "    value of tag field in file = %%t %%a %%l %%n %%y %%g %%c)\n"
        " -%s <genre>\t"    "\n"
        " -%s <comment>\t"  "\n"
        " -%s <filename>\t" "copy tags read from filename\n"
        " -%s <template>\t" "rename files according to template\n"
        " -%s <format>\t"   "print formatted string on standard output\n"
        " -%s\t\t"          "match variables in filespec\n"
        " -%s\t\t"          "search recursively\n"
        " -%s\t\t"          "disable internal wildcard handling\n"
        " -%s\t\t"          "preserve modification time of files\n"
        " -%s\t\t"          "list all recognized id3v1 genres\n"
        " -%s\t\t"          "print version info\n"
        "Only on last selected tag type:\n"
        " -%s <size>  \t"   "set tag size\n"
        " -%s\t\t"          "only write if tag already exists\n"
        " -%s\t\t"          "update all standard fields\n"
        " -%sTYPE\t\t"      "remove `TYPE' frames\n"
        " -%sTYPE <data>\t" "write a `TYPE' frame\n"
        "\nReport bugs to <squell@alumina.nl>.\n",
        Name,
        Name,
        flags[ 0], flags[ 2], flags[ 4], flags[ 6], flags[ 8], flags[10], flags[12], flags[14], flags[16], flags[18],
        flags[20], flags[22], flags[24], flags[26], flags[28], flags[30], flags[32], flags[34], flags[36], flags[38],
        flags[40], flags[42], flags[44]
    );
    exit(exitc=1);
}

static int shelp(bool quit = true)
{
    fprintf(stderr, "Try `%s -h' or `%s --help' for more information.\n", Name, Name);
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

static const char* cmdalias(const char* arg)
{
    for(size_t i=1; i < sizeof Options/sizeof(const char*); i+=2) {
        if(strcmp(arg,Options[i]) == 0) {
            return Options[i-1];
        }
    }
    eprintf("unrecognized switch `-%s'\n", arg);
    return shelp(), arg;
}

static long argtol(const char* arg)            // convert argument to long
{
    char* endp;
    long n = strtol(arg, &endp, 10);
    if(*endp != '\0' || n < 0) {
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

public:
    void without_globbing(const char* name)
    {
        fileexp::record dummy = { };
        strncpy(dummy.path, name, PATH_MAX-1);
        file(dummy.path, dummy);
    }
};

bool    verbose::enable;
clock_t verbose::time;

/* ====================================================== */

namespace op {

    enum {                                     // state information bitset
        no_op  =  0x00,
        recur  =  0x01,                        // work recursively?
        w      =  0x02,                        // write  requested?
        ren    =  0x04,                        // rename requested?
        rd     =  0x08,                        // read   requested?
        clobr  =  0x10,                        // clear  requested?
        patrn  =  0x20,                        // match  requested?
        noglob =  0x40                         // disable wildcards?
    };
    typedef int oper_t;

    template<class T> struct box { T object; };
    template<class T> T& use(box<T>& x) { return x.object; }
    template<class T> T& use(T& x) { return x; }

    struct tag_info :
      out::query,
      box<out::ID3>,
      box<out::ID3v2>,
      box<out::Lyrics3>,
      box< tag::combined< tag::reader > >,
      tag::reader,
      out::file,
      fileexp::find
    {
        template<class T>
        T& enable()
        {
            T& selected = use<T>(*this);
            use< tag::combined<tag::handler> >(*this).with(selected);
            use< tag::combined<tag::reader>  >(*this).with(selected);
            return selected;
        }

        tag::metadata* read(const char* fn) const
        { return box< tag::combined<tag::reader> >::object.read(fn); }

        bool file(const char*, const fileexp::record& rec)
        {
            tag::output(use< tag::combined<tag::reader> >(*this), rec.path, stdout);
            return true;
        }
    };

    inline tag::handler* operator|(tag::handler* x, tag_info& y)
    {
        return x? x : &y;
    }

}

/* ====================================================== */

  // tag lister

struct listtag : fileexp::find {
    listtag(tag::reader& in) : m_reader(in)
    { }
    tag::reader& m_reader;

    static void content(const char* fmt, tag::metadata::value_string data)
    {
        if(data.good()) {
            string str = data;
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

  // performs the selected operations on the file arguments

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
    const char*   val[FIELD_MAX] = { 0, };     // fields to alter

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
            if(*opt == '\0' && argv[i][0] == '-')
                opt = argv[i]+1;
            else --i;                          // stash argument for later

            switch( *opt++ ) {
            case 'v': verbose::enable=1;  break;
            case 'M': tag.touch(false);   break;
            case 'f': cmd = set_rename;   break;
            case 'q': cmd = set_query;    break;

            case 'm': state |= patrn;  break;
            case 'R': state |= recur;  break;
            case 'X': state |= noglob; break;

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
                    char tmp[2] = { opt[-1] };
                    cmdalias(tmp);             // will produce error
                }
                cmd = std_field; break;
            case '3':
                chosen = &tag.enable<out::Lyrics3>().create();
                break;
            case '1':
                chosen = &tag.enable<out::ID3>().create();
                break;
            case '2':
                chosen = &tag.enable<out::ID3v2>().create();
                break;
            case '0':
                chosen = &tag;             // "null" tag - does nothing
                break;

            case 's':                      // tag specific switches
                if(*opt == '\0')
                    cmd = suggest_size;
                else {
                    long n = argtol(opt);
                    (chosen | tag)->reserve(n);
                    state |= w;
                }
                opt = none;
                break;

            case 'u':
                for(int j = 0; j < FIELD_MAX; ++j)
                    (chosen | tag)->set(ID3field(j), mass_tag::var(j));
                state |= w;
                break;

            case 'E':
                (chosen | tag)->create(false);
                break;

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

                eprintf("specify tag format before -%c\n", opt[-1]);
                shelp();

            case 'L':
                for(int j=0; j < ID3v1_numgenres; ++j) {
                    printf("%3d: %s\n", j+1, ID3v1_genre[j]);
                }
                return 0;

            case '?': Help(1);
            case 'h': Help();
            case 'V': Copyright();

            case '-':
                if(opt-2 != argv[i]) {
                    cmdalias(opt-2);           // will produce error
                } else if(*opt == '\0') {
            case '\0':                         // end of switches
                    cmd = force_fn;
                } else {                       // --long-option
                    char *sep = strchr(opt, '=');
                    if(sep) {
                        const char save = sep[1];
                        sep[1] = '\0';         // this is a kludge
                        sep[0] = *cmdalias(argv[i]+1);
                        sep[1] = save;
                        *(argv[i] = sep-1) = '-';;
                    } else {
                        strcpy(argv[i]+1, cmdalias(argv[i]+1));
                    }
                    --i;
                }
                opt = none;
            }
            continue;

        case std_field:                        // write a standard field
            val[field] = argv[i];
            break;

        case set_copyfrom:                     // specify source tag
            copyfn = argv[i];
            state |= clobr;
            break;

        case custom_field:                     // v2 - write a custom field
            if(! chosen->set(opt, argv[i]) ) {
                eprintf("writing `%s' frames is not supported\n", opt);
                shelp();
            }
            opt = none;
            break;

        case suggest_size:                     // v2 - suggest size
            (chosen | tag)->reserve( argtol(argv[i]) );
            break;

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

        case force_fn:                         // argument is filespec
            if(!chosen) {                      // use default tags
                tag.enable<out::ID3v2>();
                tag.enable<out::Lyrics3>();
                tag.enable<out::ID3>().create();
            }

            for(int f = 0; f < FIELD_MAX; ++f) // propagate general ops
                if(val[f]) tag.set(ID3field(f), val[f]);
            if(copyfn && !tag.from(copyfn))
                eprintf("note: could not read tags from %s\n", copyfn);
            if(state & clobr)
                tag.rewrite();

            #define is_combi(x) ((x) & (x)-1)  // test for pure powers of two
            if(is_combi(state & (patrn|recur|noglob))) {
                eprintf("can only use one of -R, -m and -X at the same time\n");
                shelp();
            }

            if(state & patrn) {
                if(argv[i+1]) {
                    eprintf("-m %s: no file arguments are allowed\n", argv[i]);
                    shelp();
                }
                pattern spec(tag, argpath(argv[i]));
                if(spec.vars() > 0) state |= w;
                strcpy(argv[i], spec.c_str());
            }

            tag::writer* selected = &use<out::file>(tag);
            tag::reader* source   = &tag;

            switch(state & (w|rd|ren)) {
            case op::rd:
                selected = &use<out::query>(tag);
            case op::ren:
                tag.ignore(0, tag.size());     // don't perform no-ops
            case op::w | op::ren:
            case op::w:
                break;
            default:
                eprintf("cannot combine -q with any modifying operation\n");
                shelp();
            case op::no_op:
                if(verbose::enable)
                    return process_(tag, &argv[i], state & recur);
                listtag viewer(*source);
                return process_(viewer, &argv[i], state & recur);
            }

            verbose tagger(*selected, *source);
            if(state & noglob) {
                tagger.without_globbing(argv[i]);
                if(argv[i+1]) continue; else return exitc;
            } else {
                return process_(tagger, &argv[i], state & recur);
            }
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
        static struct chcp {                  // fiddle with the console fonts
            int const cp_in, cp_out;
            chcp(int cp_new = GetACP())
            : cp_in(GetConsoleCP()), cp_out(GetConsoleOutputCP())
            { SetConsoleCP(cp_new), SetConsoleOutputCP(cp_new); }
            ~chcp()
            { SetConsoleCP(cp_in),  SetConsoleOutputCP(cp_out); }
        } lock;
#  else
        setlocale(LC_CTYPE, "");
        if(mblen(0,0) != 0)
            setlocale(LC_CTYPE, "C");
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
