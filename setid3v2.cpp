#include <string>
#include <map>
#include <new>
#include <cctype>
#include <cstdlib>
#include "setid3v2.h"
#include "getid3v2.h"
#include "sedit.h"
#include "id3v1.h"
#include "id3v2.h"
#include "fileops.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Note: I'm devoting quite a bit of code to glue the interface from C to C++,
  and I'm not entirely happy about it. I'm a bit confused as to the reasons I
  didn't chose to write id3v2.c in C++. :(

*/

using namespace std;

using set_tag::ID3v2;
using set_tag::ID3field;

namespace {

/* ===================================== */

 // extra hairyness to prevent buffer overflows by re-allocating on the fly
 // overkill, but i had to do a runtime check anyway, so.

    class writer {
        size_t avail;
        char *base, *dest;

    public:
        operator char*()   { return base; }

        writer(size_t len) { base = (char*) malloc(avail=len);
                             if(!base) throw bad_alloc();
                             dest = (char*) ID3_put(base,0,0,0); }

       ~writer()           { free(base); }

        void put(const char* ID, const void* src, size_t len);
    };

    void writer::put(const char* ID, const void* src, size_t len)
    {
        static size_t factor = 0x1000;      // start reallocing in 4k blocks

        if(len+10 > avail) {
            while(len+10 > factor) factor *= 2;
            int size = dest - base;
            base     = (char*) realloc(base, size+factor);
            avail    = factor;
            if(!base) throw bad_alloc();
            dest     = base + size;         // translate current pointer
        }

        avail -= (len+10);
        dest = (char*) ID3_put(dest, ID, src, len);
    }

/* ===================================== */

 // convert C handler to a C++ exception at program startup

    extern "C" int copy_on_fail(const char*, const char*);

    struct guard {
        guard()            { ID3_wfail = copy_on_fail; }
        static string err;
        static void raise();
    } static fail_inst;

    string guard::err;

    void guard::raise()
    {
        string emsg;
        emsg.swap(err);
        if(!emsg.empty())
            throw set_tag::failure(emsg);
    }

    extern "C" int copy_on_fail(const char* oldn, const char* newn)
    {
        if(! cpfile(oldn, newn) ) {
            string emsg(" lost, new contents still in ");
            guard::err = newn + emsg + oldn;
        }
        return 1;
    }

}

/* ===================================== */

 // checks if a given field is valid

static string binarize(string field, const cvtstring& src)
{
    string s = src.latin1();

    if(field == "TCON") {                             // genre by number
        unsigned int x = atoi(s.c_str())-1;           // is portable
        if(x < ID3v1_numgenres) s = ID3v1_genre[x];
    }
    if(field[0] == 'T' || field == "IPLS" || field == "WXXX") {
 /*     bool t = field.compare(1,3,"XXX") == 0;  gcc 2.95 doesnt */
        bool t = (field[1]=='X' & field[2]=='X' & field[3]=='X');
        s.insert(string::size_type(0), 1+t, '\0');
    } else if(field[0] == 'W') {
        //
    } else if(field == "COMM" || field == "USLT" || field == "USER") {
        s.insert(string::size_type(0), "\0xxx\0", 4 + (field[3]!='R'));
    } else if(field == "PCNT") {
        unsigned long t = strtol(s.c_str(), 0, 0);
        s.erase();
        s.push_back(t >> 24 & 0xFF);
        s.push_back(t >> 16 & 0xFF);
        s.push_back(t >>  8 & 0xFF);
        s.push_back(t       & 0xFF);
    } else {
        s.erase();
    }
    return s;
}

/* ===================================== */

typedef map<string,string> db;

const static char xlat[][5] = {                     // ID3field order!
    "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
};

ID3v2& ID3v2::set(ID3field i, const char* m)
{
    if(i < FIELDS)
        set(xlat[i], m);
    return *this;
}

ID3v2& ID3v2::reserve(size_t n)
{
    resize = n? n : 1;
    return *this;
}

ID3v2& ID3v2::clear()
{
    fresh = true;
    return *this;
}

bool ID3v2::set(std::string field, std::string s)
{
    if(field.length() != 4)
        return false;
    for(int n = 0; n < 4; ++n)
        if(!isalnum(field[n] = toupper(field[n])))
            return false;
    if( binarize(field, "0").length() != 0 ) {      // test a dummy string
        mod[field] = s;                             // was: mod.insert
        return true;
    }
    return false;
}

bool ID3v2::rm(std::string field)
{
    mod[field].erase();
    return true;
}

/* ===================================== */

set_tag::reader* ID3v2::read(const char* fn) const
{
    return new read::ID3v2(fn);
}

template<void dispose(void*)> struct voidp {        // auto-ptr like
    void* data;
    operator void*() { return data;   }
    voidp(void* p)   { data = p;      }
   ~voidp()          { dispose(data); }
};

bool ID3v2::vmodify(const char* fn, const subst& v) const
{
    size_t check;
    void* buf = ID3_readf(fn, &check);

    if(!buf && check != 0)                          // evil ID3 tag
        return false;

    voidp<ID3_free> src  ( fresh? (void*)0 : buf );
    writer          tag  ( 0x1000 );
    db              table( mod );

    if( src ) {                                     // update existing tags
        ID3FRAME f;
        ID3_start(f, src);

        while(ID3_frame(f)) {
            db::iterator p = table.find(f->ID);
            if(p == table.end())
                tag.put(f->ID, f->data, f->size);
            else
                if(!p->second.empty()) {            // else: erase frames
                    string s = binarize(p->first, edit(p->second, v));
                    tag.put(f->ID, s.data(), s.length());
                    table.erase(p);
                }
        }
    }

    for(db::iterator p = table.begin(); p != table.end(); ++p) {
        if(p->second != "") {
            string s = binarize(p->first, edit(p->second, v));
            tag.put(p->first.c_str(), s.data(), s.length());
        }
    }

    bool result = ID3_writef(fn, tag, resize);
    guard::raise();

    return result;
}

