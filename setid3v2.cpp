#include <new>
#include <cctype>
#include <cstdlib>
#include "setid3v2.h"
#include "getid3v2.h"
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
        ID3VER version;

    public:
        void init(ID3VER v, size_t len)
                         { base = (char*) malloc(avail=len);
                           if(!base) throw bad_alloc();
                           dest = (char*) ID3_put(base,version=v,0,0,0); }

        operator char*() { return base; }

        writer()         { }
       ~writer()         { free(base); }

        void put(const char* ID, const void* src, size_t len);

    private:
        writer(writer&);                    // non-copyable
        void operator=(writer&);
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
        dest = (char*) ID3_put(dest, version, ID, src, len);
    }

/* ===================================== */

 // convert C handler to a C++ exception at program startup

    extern "C" void copy_failure(const char*, const char*);

    struct guard {
        guard()            { ID3_wfail = copy_failure; }
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

    extern "C" void copy_failure(const char* oldn, const char* newn)
    {
        string emsg(" lost, new contents still in ");
        guard::err = newn + emsg + oldn;
    }

}

/* ===================================== */

 // code for constructing ID3v2 frames. rather hairy, but hey, ID3v2 sucks

static string binarize(const string field, cvtstring content)
{
    if(field == "TCON" || field == "TCO") {                // genre by number
        unsigned int x = atoi(content.latin1().c_str())-1; // is portable
        if(x < ID3v1_numgenres) content = ID3v1_genre[x];
    }

    using set_tag::read::ID3v2;

    string data;
    if(!ID3v2::is_valid(field))
        return data;
    if(ID3v2::is_counter(field)) {
        unsigned long t = strtol(content.latin1().c_str(), 0, 0);
        data.push_back(t >> 24 & 0xFF);
        data.push_back(t >> 16 & 0xFF);
        data.push_back(t >>  8 & 0xFF);
        data.push_back(t       & 0xFF);
        return data;
    }
    const char nul[2] = { 0 };
    data = char(0);                    // unicode to be implemented
    if(ID3v2::has_lang(field))
        data.append("xxx");
    if(ID3v2::has_desc(field))
        data.append(""), data.append(nul, 1);

    if(data.length() > 1 || ID3v2::is_text(field)) {
        return data + content.latin1();
    } else if(ID3v2::is_url(field)) {
        return content.latin1();
    } else {
        return string();
    }
}

/* ===================================== */

typedef map<string,string> db;

ID3v2& ID3v2::set(ID3field i, string m)
{
    const static char xlat2[][4] = {                     // ID3field order!
        "TT2", "TP1", "TAL", "TYE", "COM", "TRK", "TCO"
    };
    const static char xlat3[][5] = {
        "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
    };
    if(i < FIELDS) {
        set(xlat2[i], m);      // let error handling decide between them.
        set(xlat3[i], m);
    }
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

bool ID3v2::set(string field, string s)
{
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

bool ID3v2::vmodify(const char* fn, const subst& v) const
{
    size_t check;
    void* buf = ID3_readf(fn, &check);

    if(!buf && check != 0)                          // evil ID3 tag
        return false;

    struct wrapper {
        void* data;
        operator void*() { return data;  }
       ~wrapper()        { ID3_free(data); }
    };
    wrapper src = { fresh? (void*)0 : buf };
    writer  tag;
    db      table ( mod );

    if( src ) {                                     // update existing tags
        ID3FRAME f;
        tag.init(ID3_start(f, src), (check+0xFFF)&~0xFFF);

        while(ID3_frame(f)) {
            db::iterator p = table.find(f->ID);
            if(p == table.end())
                tag.put(f->ID, f->data, f->size);
            else {
                cvtstring s = edit(p->second, v);
                if(!s.empty()) {                    // else: erase frames
                    string b = binarize(p->first, s);
                    tag.put(f->ID, b.data(), b.length());
                    table.erase(p);
                }
            }
        }
    } else {
        tag.init(ID3_v2_3, 0x1000);                 // ID3v2.3 per default
    }

    for(db::iterator p = table.begin(); p != table.end(); ++p) {
        cvtstring s = edit(p->second, v);
        if(!s.empty()) {
            string b = binarize(p->first, s);
            tag.put(p->first.c_str(), b.data(), b.length());
        }
    }

    bool result = ID3_writef(fn, tag, resize);
    guard::raise();                                 // deferred exception?

    return result;
}

