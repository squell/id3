#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <string>
#include "fileexp.h"

#include "sedit.h"
#include "set_base.h"
#include "setid3.h"
#include "setfname.h"
#include "setecho.h"
#ifndef NO_V2
#    include "setid3v2.h"
#endif

#define _version_ "0.76-dev (2005xxx)"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static char*     name  = "id3";
static int       exitc = 0;

static void eprintf(const char* msg, ...)
{
    exitc = 2;
    va_list args;
    va_start(args, msg);
    fprintf  (stderr, "%s: ", name);
    vfprintf (stderr, msg, args);
    va_end(args);
}

static void shelp()
{
    fprintf(stderr, "Try `%s -h' for more information.\n", name);
    exit(exitc=1);
}

#include "verbose.cpp"

/* ====================================================== */

namespace {
    static set_tag::ID3field char_as_ID3field(char c)
    {
         switch(c) {                           // map from chars <-> ID3field
         case 't': return set_tag::title;
         case 'a': return set_tag::artist;
         case 'l': return set_tag::album;
         case 'y': return set_tag::year;
         case 'c': return set_tag::cmnt;
         case 'n': return set_tag::track;
         case 'g': return set_tag::genre;
         default : return set_tag::FIELDS;
         }
    }

    const char ID3field_as_char[] = "talycng";

    inline static char* ID3field_as_asciiz(int i)
    {
        static char fmt[] = "%_";
        fmt[1] = ID3field_as_char[i];
        return fmt;
    }
}

/* ====================================================== */

namespace {

    using set_tag::ID3field;
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
        uses()
        { delegate( object.active(false) ); }

        T object;
    };

    // next function acts like a cast operator on the above

    template<class T> inline T& with(uses<T>& box)             { return box.object; }
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

 // adaptation of filefindexp with verbose output

class mass_tag : public fileexp::find {
public:
    mass_tag() : recursive(0), edir(0) { }

    void operator()( const set_tag::handler&, const char* spec, const set_tag::provider& );
    bool recursive;

    class substvars;
    class r_vector;

private:
    const set_tag::handler*  tag;
    const set_tag::provider* info;
    bool edir;

    virtual void file(const char* name, const fileexp::record&);
    virtual bool dir (const char* path);
};

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

class mass_tag::substvars {
public:
    cvtstring operator[](char field) const;

    substvars(const set_tag::provider& proto, const char* fn)
    : tag(proto), filename(fn), tag_data(0) { }
   ~substvars() { delete tag_data; }

private:
    mutable const set_tag::reader* tag_data;
    static unsigned counter;

    const set_tag::provider& tag;
    const char* const filename;
};

 // range checked, boxed, constrained vector

class mass_tag::r_vector {
    const vector<string>& vec;
public:
    r_vector(const vector<string>& v) : vec(v) { }
    inline const string& operator[](size_t i) const
    {
        if(i >= vec.size())
          throw out_of_range("variable index out of range");
        return vec[i];
    }
};

void mass_tag::operator()(const set_tag::handler& h, const char* spec, const set_tag::provider& p)
{
    tag  = &h;
    info = &p;
    if(! fileexp::find::operator()(spec) )
        eprintf("no %s matching %s\n", edir? "files" : "directories", spec);
}

bool mass_tag::dir(const char* path)
{
    verbose.reportd(path);
    edir = true;
    return recursive;
}

void mass_tag::file(const char* name, const fileexp::record& f)
{
    verbose.reportf(name);
    if(! tag->modify(f.path, r_vector(f.var), substvars(*info, f.path)) )
        return (void) eprintf("could not edit tag in %s\n", f.path);
}

unsigned mass_tag::substvars::counter = 0;

cvtstring mass_tag::substvars::operator[](char c) const
{
    switch( c ) {
        ID3field i;
        char buf[11];
    default:
        i = char_as_ID3field(c);
        if(i < set_tag::FIELDS) {
            if(!tag_data) tag_data = tag.read(filename);
            return (*tag_data)[i];
        }
        break;
    case 'x':
        counter = (counter+1) & 0xFFFF;
        sprintf(buf, "%u", counter);
        return cvtstring::latin1(buf);
    case 'F':
        return filename;
    case 'f':
        if(const char* p = strrchr(filename,'/'))
              return p+1;
          else
              return filename;
    };
    eprintf("unknown variable: %%%c\n", c);
    shelp();
}

/* ====================================================== */

static void Help()
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
        " -m <pattern>\t"  "match fields to pattern\n"
        " -q <string>\t"   "print string on stdout\n"
        " -R\t\t"          "recurse into directories\n"
        " -v\t\t"          "give verbose output\n"
        " -V\t\t"          "print version info\n"
#ifndef NO_V2
        "Only on last selected tag:\n"
        " -rXXXX\t\t"      "erase all XXXX frames\n"
        " -wXXXX <data>\t" "write a XXXX frame\n"
        " -s <size>\t"     "force size of final tag\n"
#endif
        " -!\t\t"          "force rewrite of tag\n"
        "\nReport bugs to <squell@alumina.nl>.\n",
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

inline static char* argpath(char* arg)
{
#if defined(__DJGPP__) || defined(__WIN32__)
    for(char* p = arg; *p; ++p)                // convert backslashes
        if(*p == '\\') *p = '/';
#endif
    return arg;
}

static void defaults(metadata& tag, set_tag::handler*& target, set_tag::provider*& source)
{
    if(!target) target = &with<ID3>(tag).active(true);
    if(!source) source = &with<ID3>(tag);
}

/* ====================================================== */

namespace op {                                 // state information bitset
    enum oper_t {
        no_op =  0x00,
        scan  =  0x01,                         // checking for no-file args?
        w     =  0x02,                         // write  requested?
        ren   =  0x04,                         // rename requested?
        rd    =  0x08,                         // read   requested?
        recur =  0x10,                         // recursion requested?
    };
    oper_t  operator %(oper_t x,  int y) { return oper_t(x & y);    }
    oper_t& add(oper_t& x, oper_t y)     { return x = oper_t(x|y);  }
    oper_t& rem(oper_t& x, int y)        { return x = oper_t(x&~y); }
}

/* ====================================================== */

 // "inside out" way of specifying what you want.
 // - kind of lazy. but hey long live code reuse :)
 // - assumes sedit() processes variables left-to-right

struct setpattern {
    setpattern(metadata& _tag, char*& arg);    // call as if a function
    operator op::oper_t() const { return state; }
    const char* operator[](unsigned);
    const char* operator[](char c);
private:
    metadata&  tag;
    op::oper_t state;
    char       match[3];
};

setpattern::setpattern(metadata& _tag, char*& arg)
: tag(_tag), state(op::no_op)
{
    string::size_type pos(0);                  // replace '*' with stubs
    string s(arg);
    while((pos = s.find('*',pos)) != string::npos) {
        s.replace(pos, 1, "%x");
    }
    strcpy(match, "%0");
    strcpy(arg, sedit(s, *this, *this).c_str());
}

const char* setpattern::operator[](char c)
{
    if(match[1]++ == '9') {                    // limit reached
        eprintf("too many variables in pattern\n");
        shelp();
    }
    ID3field field = char_as_ID3field(c);
    if(field < set_tag::FIELDS) {
        add(state, op::w);   tag.set(field, match);
    } else if(c == 'f') {
        add(state, op::ren); tag.enable<filename>()->rename(match);
    } else if(c == 'x') {
        ;                                      // pass over in silence
    } else {
        eprintf("illegal variable: %%%c\n", c);
        shelp();
    }
    return "*";
}

const char* setpattern::operator[](unsigned)
{
    eprintf("illegal use of variable index\n");
    shelp();
}

/* ====================================================== */

int main_(int argc, char *argv[])
{
    set_tag::echo display;
    metadata tag;
    mass_tag apply;

    ID3field field;
    string fieldID;                            // free form field selector

    set_tag::provider* source = 0;             // pointer to first enabled
    set_tag::handler*  chosen = 0;             // pointer to last enabled

    using namespace op;
    enum parm_t {                              // parameter modes
        no_value, force_fn, pattern_fn,
        stdfield, customfield, suggest_size,
        set_rename, set_query,
    };

    parm_t cmd   = no_value;
    oper_t state = scan;
    char*  opt   = "";                         // used for command stacking
    bool   check = false;                      // foolproof check

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command argument
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-' && scan) opt = argv[i]+1;
            if(*opt == '\0') {
        case force_fn:                         // argument is filespec
                defaults(tag, chosen, source);
                argpath(argv[i]);
                rem(state, scan);
                if(state%(w|rd) == w)          // no-op check
                    apply(tag, argv[i], *source);
                else if(state%(ren|rd) == ren)
                    apply(with<filename>(tag), argv[i], *source);
                else if(state == rd)
                    apply(display, argv[i], *source);
                else if(!state)
                    eprintf("nothing to do with %s\n", argv[i]);
                else {
                    eprintf("incompatible operation requested\n");
                    shelp();
                }
            } else {
                switch( *opt++ ) {             // argument is a switch
                case 'v': verbose.on(); break;
                case 'R': apply.recursive = true; break;
                case 'm': cmd = pattern_fn; break;
                case 'f': cmd = set_rename; break;
                case 'q': cmd = set_query;  break;
                case 'd': tag.clear(); add(state, w); break;
                case 't':
                case 'a':
                case 'l':
                case 'y':
                case 'c':
                case 'g':
                case 'n':
                    field = char_as_ID3field(opt[-1]); cmd = stdfield; break;
#ifndef NO_V2
                case '1':
                    chosen = tag.enable<ID3>();
                    if(!source) source = &with<ID3>(tag);
                    break;
                case '2':
                    chosen = tag.enable<ID3v2>();
                    if(!source) source = &with<ID3v2>(tag);
                    break;

                case 's':                      // tag specific switches
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
                        chosen->rm(opt); add(state, w);
                        opt = "";
                        break;
                    }
#endif
                case '!':
                    if(chosen) {
                        if(check) {
                            eprintf("-! must come before any fields\n");
                            shelp();
                        }
                        for(int i = 0; i < set_tag::FIELDS; ++i)
                            chosen->set(ID3field(i), ID3field_as_asciiz(i));
                        add(state, w);
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
                default:
                    eprintf("unrecognized switch: -%c\n", opt[-1]);
                    shelp();
                }
            }
            continue;

        case stdfield:                         // write a standard field
            tag.set(field, argv[i]);
            check = true;
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
            check = true;
            break;
#endif
        case pattern_fn:                       // filename pattern shorthand
            add(state, setpattern(tag, argv[i--]));
            cmd = force_fn;
            continue;

        case set_rename:                       // specify rename format
            if(strrchr(argv[i],'/')) {
                eprintf("will not rename across directories\n");
                shelp();
            } else if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
                shelp();
            } else
                tag.enable<filename>()->rename(argpath(argv[i]));
            add(state, ren);
            cmd = no_value;
            continue;

        case set_query:                        // specify echo format
            if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
                shelp();
            } else
                display.format(argv[i]);
            add(state, rd);
            cmd = no_value;
            continue;
        };

        add(state, w);                         // set operation done flag
        cmd = no_value;
    }

    if(state % scan)
        eprintf("missing file arguments\n");
    if(state == scan)
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

