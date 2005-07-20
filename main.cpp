#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>
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

#define _version_ "0.76 (2005201)"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

 // this file is definately too complicated - it should be split up into
 // seperate  components that are agnostic of eachother.

/* ====================================================== */

 // exitcodes: 0 - ok, 1 - syntax, 2 - errors, 3 - fatal errors

static const char* Name  = "id3";
static int         exitc = 0;

static void eprintf(const char* msg, ...)
{
    exitc = 2;
    va_list args;
    va_start(args, msg);
    fprintf  (stderr, "%s: ", Name);
    vfprintf (stderr, msg, args);
    va_end(args);
}

static void shelp()
{
    fprintf(stderr, "Try `%s -h' for more information.\n", Name);
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

   // this template class:
   // - boxes a handler to make it safe for (multiple) inheritance,

    template<class T> struct uses {
        T object;
    };
}

struct metadata :                    // owns the data it contains
  set_tag::combined,
#ifndef NO_V2
  uses<ID3v2>,
#endif
  uses<ID3>,
  uses<set_tag::filename>,
  uses<set_tag::echo>
{
    string format;                   // for free format 'pseudo formats'

    template<class Tag> Tag* activate()
    { delegate(uses<Tag>::object);
      return &this->uses<Tag>::object; }
};

/* ====================================================== */

 // implementation of fileexp::find using set_tag objects

class mass_tag : public fileexp::find {
public:
    inline mass_tag(const set_tag::handler&, const set_tag::provider&);

    class substvars;
    class r_vector;

    const set_tag::handler&  tag_update;
    const set_tag::provider& tag_info;
private:
    unsigned counter;

    virtual bool file(const char* name, const fileexp::record&);
    virtual bool dir (const fileexp::record&);
};

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

class mass_tag::substvars {
public:
    cvtstring operator[](char field) const;

    substvars(const set_tag::provider& info, const char* fn, unsigned x)
    : tag_data(0), tag(&info), filename(fn), num(x) { }
   ~substvars()
    { delete tag_data; }

private:
    mutable const set_tag::reader* tag_data;

    const set_tag::provider* const tag;
    const char*              const filename;
    unsigned int             const num;

    substvars(const substvars&);       // don't copy
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

 //

mass_tag::mass_tag(const set_tag::handler& writer, const set_tag::provider& reader)
: tag_update(writer), tag_info(reader), counter(0)
{
}

bool mass_tag::dir(const fileexp::record& d)
{
    verbose.reportd(d.path);
    return (counter = 1);
}

bool mass_tag::file(const char* name, const fileexp::record& f)
{
    substvars letter_vars(tag_info, f.path, counter++);
    r_vector  number_vars(f.var);

    verbose.reportf(name);
    if(! tag_update.modify(f.path, number_vars, letter_vars) )
        return eprintf("could not edit tag in %s\n", f.path), false;

    return true;
}

cvtstring mass_tag::substvars::operator[](char c) const
{
    switch( c ) {
        ID3field i;
        char buf[11];
    default:
        i = char_as_ID3field(c);
        if(i < set_tag::FIELDS) {
            if(!tag_data) tag_data = tag->read(filename);
            return (*tag_data)[i];
        }
        break;
    case 'x':
        sprintf(buf, "%u", num & 0xFFFFu);
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

 // adapter/decorator for fileexp::find that handles general recursion

class recursive_find : public fileexp::find {
public:
    const char*    const mask;
    fileexp::find& backend;
    inline recursive_find(fileexp::find&, const char*);
private:
    virtual bool file(const char*, const fileexp::record&);
};

recursive_find::recursive_find(fileexp::find& search, const char* pattern)
: backend(search), mask(pattern)
{
}

bool recursive_find::file(const char* name, const fileexp::record& f)
{
    return backend.pattern(f.path, mask);
}

/* ====================================================== */

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
        " -n <tracknr>\t"  "\t file name/counter          = %%f %%x\n"
        " -y <year>\t"     "\t value of tag field in file = %%t %%a %%l %%n %%y %%g %%c)\n"
        " -g <genre>\t"    "\n"
        " -c <comment>\t"  "\n"
        " -f <template>\t" "rename files according to template\n"
        " -q <format>\t"   "print formatted string on standard output\n"
        " -m\t\t"          "match variables in filespec\n"
        " -R <pattern>\t"  "search directories recursively for wildcard pattern\n"
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

static void Copyright()
{
 //      |=======================64 chars wide==========================|
    printf(
        "%s " _version_ ", Copyright (C) 2003, 04, 05 Marc R. Schoolderman\n"
        "This program comes with ABSOLUTELY NO WARRANTY.\n\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; see the file named COPYING in the\n"
        "source distribution for details.\n",
        Name
    );
    exit(exitc=1);
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

namespace op {                                 // state information bitset
    enum {
        no_op =  0x00,
        scan  =  0x01,                         // checking for no-file args?
        w     =  0x02,                         // write  requested?
        ren   =  0x04,                         // rename requested?
        rd    =  0x08,                         // read   requested?
        patrn =  0x10,                         // match  requested?
    };
    typedef int oper_t;
}

/* ====================================================== */

 // "inside out" way of specifying what you want.
 // - kind of lazy. but hey long live code reuse :)
 // - assumes sedit() processes variables left-to-right

struct setpattern {
    setpattern(metadata& _tag, char*& _arg);   // call as if a function
    operator op::oper_t() const { return state; }
    const char* operator[](unsigned);
    const char* operator[](char c);
private:
    metadata&  tag;
    char*&     arg;
    op::oper_t state;
    char       match[3];
};

setpattern::setpattern(metadata& _tag, char*& _arg)
: tag(_tag), arg(_arg), state(op::no_op)
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
        eprintf("in %s: too many variables in pattern\n", arg);
        shelp();
    }
    ID3field field = char_as_ID3field(c);
    if(field < set_tag::FIELDS) {
        state |= op::w; tag.set(field, match);
    } else if(c == 'x') {
        ;                                      // pass over in silence
    } else {
        eprintf("in %s: illegal variable %%%c\n", arg, c);
        shelp();
    }
    return "*";
}

const char* setpattern::operator[](unsigned i)
{
    eprintf("in %s: illegal variable %%%c\n", arg, (i+1) % 10 + '0');
    shelp();
}

/* ====================================================== */

  // dynamically create a suitably initialized function object

struct null_op : fileexp::find {
    bool file(const char*, const fileexp::record& r)
    { return eprintf("nothing to do with %s\n", r.path), true; }
};

static fileexp::find* instantiate(op::oper_t state, metadata& tag,
  const set_tag::provider* src_ptr, const char* recurse)
{
    if(!src_ptr) src_ptr = tag.activate<ID3>();
    using namespace op;

    set_tag::handler* selected;
    static null_op dummy;

    state &= w|rd|ren;

    if(state%w == 0 && tag.handlers().size() != 1)
        eprintf("multiple selected tags ignored when only reading\n");

    if(state%ren)                              // add rename as final
        tag.activate<set_tag::filename>()->rename(tag.format);

    fileexp::find* routine = &dummy;

    do {
        if(!state)
            break;
        else if(state%(w|rd) == w)
            selected = &tag;
        else if(state%(ren|rd) == ren)             // optimization
            selected = &tag.uses<set_tag::filename>::object;
        else if(state == rd)
            selected = &tag.uses<set_tag::echo>::object.format(tag.format);
        else {
            eprintf("cannot combine -q with any modifying operation\n");
            shelp();
        }
        routine = new mass_tag(*selected, *src_ptr);
    } while(0);

    if(!recurse)
        return routine;
    else
        return new recursive_find(*routine, recurse);
}

  // contains CLI interface loop

int main_(int argc, char *argv[])
{
    metadata tag;

    ID3field field;

    set_tag::provider* source  = 0;            // pointer to first enabled
    set_tag::handler*  chosen  = 0;            // pointer to last enabled
    const char*        recmask = 0;            // path pattern (if recursive)

    using namespace op;
    char none[1] = "";

    enum parm_t {                              // parameter modes
        no_value, force_fn, recurse_expr, pattern_fn,
        std_field, custom_field, suggest_size,
        set_rename, set_query
    };

    parm_t cmd   = no_value;
    oper_t state = scan;
    char*  opt   = none;                       // used for command stacking

    for(int i=1; i < argc; i++) {
        switch( cmd ) {
        case no_value:                         // process a command argument
            if(*opt != '\0') --i; else
                if(argv[i][0] == '-' && scan) opt = argv[i]+1;
            if(*opt == '\0') {
        case force_fn:                         // argument is filespec
                argpath(argv[i]);
                if(state & patrn)              // filename pattern shorthand
                    state |= setpattern(tag, argv[i]);

                static fileexp::find& apply
                  = *instantiate(state&=~scan, tag, source, recmask);

                if(! apply.glob(argv[i]) ) {
                    if(!recmask)
                        eprintf("no files matching %s\n", argv[i]);
                    else
                        eprintf("no files matching `%s' in %s\n", recmask, argv[i]);
                }
            } else {
                switch( *opt++ ) {             // argument is a switch
                case 'v': verbose.on(); break;
                case 'm': if(recmask) {
                              eprintf("cannot use -R and -m at the same time\n");
                              shelp();
                          }
                          state |= patrn;     break;
                case 'R': cmd = recurse_expr; break;
                case 'f': cmd = set_rename;   break;
                case 'q': cmd = set_query;    break;
                case 'd': tag.clear(); state |= w; break;
                case 't':
                case 'a':
                case 'l':
                case 'y':
                case 'c':
                case 'g':
                case 'n':
                    field = char_as_ID3field(opt[-1]); cmd = std_field; break;
#ifndef NO_V2
                case '1':
                    chosen = tag.activate<ID3>();
                    if(!source) source = &tag.uses<ID3>::object;
                    break;
                case '2':
                    chosen = tag.activate<ID3v2>();
                    if(!source) source = &tag.uses<ID3v2>::object;
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
                case '!':             // deprecated
                    eprintf("-! will be removed, use -u instead");
                case 'u':
                    if(chosen) {
                        for(int i = 0; i < set_tag::FIELDS; ++i)
                            chosen->set(ID3field(i), ID3field_as_asciiz(i));
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
                default:
                    eprintf("-%c: unrecognized switch\n", opt[-1]);
                    shelp();
                }
            }
            continue;

        case std_field:                        // write a standard field
            tag.set(field, argv[i]);
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
                tag.format = argpath(argv[i]);
            state |= ren;
            cmd = no_value;
            continue;

        case set_query:                        // specify echo format
            if(*argv[i] == '\0') {
                eprintf("empty format string rejected\n");
                shelp();
            } else
                tag.format = argv[i];
            state |= rd;
            cmd = no_value;
            continue;

        case recurse_expr:                     // enable recursion (ugh)
            if(state & patrn) {
                eprintf("cannot use -R and -m at the same time\n");
                shelp();
            }
            recmask = argpath(argv[i]);
            cmd = no_value;
            continue;

        };

        state |= w;                            // set operation done flag
        cmd = no_value;
    }

    if(state & scan) {                         // code duplication is awful.
        if(!recmask)
            eprintf("missing file arguments\n");
        else if(! instantiate(state, tag, source, recmask)->glob(".") ) {
            eprintf("no files matching `%s' in %s\n", recmask, ".");
        }
    }
    if(state == scan && !recmask)
        shelp();

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
    } catch(const set_tag::failure& f) {
        eprintf("%s (tagging aborted)\n", f.what());
    } catch(const out_of_range& x) {
        eprintf("%s\n", x.what());
    } catch(const exception& exc) {
        eprintf("unhandled exception: %s\n", exc.what());
    } catch(...) {
        eprintf("unexpected unhandled exception\n");
    }
    return 3;
}

