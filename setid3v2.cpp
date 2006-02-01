#include <new>
#include <algorithm>
#include <functional>
#include <cctype>
#include <cstdlib>
#include "char_ucs.h"
#include "id3v1.h"
#include "id3v2.h"
#include "fileops.h"
#include "getid3v2.h"
#include "setid3v2.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

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
        ID3VER version;                     // writes ID3v2.3 per default

    public:
        void init(size_t len, ID3VER v = ID3_v2_3)
                         { base = (char*) malloc(avail=len+!len);
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
 // returns empty string if unsupported

static string binarize(const string field, charset::conv<charset::latin1> content)
{
    if(field == "TCON" || field == "TCO") {                // genre by number
        unsigned int x = atoi(content.c_str())-1;          // is portable
        if(x < ID3v1_numgenres) content = ID3v1_genre[x];
    }

    using set_tag::read::ID3v2;
    using charset::conv;

    string data;
    if(!ID3v2::is_valid(field))
        return data;
    if(ID3v2::is_counter(field)) {
        unsigned long t = strtol(content.c_str(), 0, 0);
        data.push_back(t >> 24 & 0xFF);
        data.push_back(t >> 16 & 0xFF);
        data.push_back(t >>  8 & 0xFF);
        data.push_back(t       & 0xFF);
        return data;
    }

    const wstring& ws = content.str<wchar_t>();
    const char nul[2] = { 0 };

    data = char(ws.end() != find_if(ws.begin(), ws.end(),
                                      bind2nd(greater<wchar_t>(), 0xFF)));
    if(ID3v2::has_lang(field))
        data.append("xxx");
    if(ID3v2::has_desc(field))             // desc fields to be implemented
        data.append(nul, 1);

    if(data.length() > 1 || ID3v2::is_text(field)) {
        return data + (data[0]==0 || ID3v2::is_url(field) ?
                         content.str() : conv<charset::ucs2>(content).str());
    } else if(ID3v2::is_url(field)) {
        return content;
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

bool ID3v2::from(const char* fn)
{
    ID3_free(null_tag);
    return null_tag = (fn? ID3_readf(fn, 0) : 0);
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

bool ID3v2::vmodify(const char* fn, const set_tag::function& edit) const
{
    struct wrapper {
        void* data;
        operator void*() { return data;  }
       ~wrapper()        { ID3_free(data); }
    };

    size_t check;
    wrapper buf  = { ID3_readf(fn, &check) };

    if(!buf && check != 0)                          // evil ID3 tag
        return false;

    const void* src = fresh? null_tag : (void*)buf;
    writer tag;
    db table(mod);

    if( src ) {                                     // update existing tags
        ID3FRAME f;
        tag.init(0x1000, ID3_start(f, src));

        while(ID3_frame(f)) {
            db::iterator p = table.find(f->ID);
            if(p == table.end())
                tag.put(f->ID, f->data, f->size);
            else {
                if(function::result s = edit(p->second)) {
                    if(!s.empty()) {                // else: erase frames
                        string b = binarize(p->first, s);
                        tag.put(f->ID, b.data(), b.length());
                        table.erase(p);
                    }
                } else {
                    tag.put(f->ID, f->data, f->size);
                }
            }
        }
    } else {
        tag.init(0x1000);
    }

    for(db::iterator p = table.begin(); p != table.end(); ++p) {
        charset::conv<> s = edit(p->second);
        if(!s.empty()) {
            string b = binarize(p->first, s);
            tag.put(p->first.c_str(), b.data(), b.length());
        }
    }

    bool result = ID3_writef(fn, tag, resize);
    guard::raise();                                 // deferred exception?

    return result;
}

ID3v2::~ID3v2()
{
    if(null_tag) ID3_free(null_tag);
}

