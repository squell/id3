#include <string>
#include <map>
#include <new>
#include <cctype>
#include <cstdlib>
#include "setid3v2.h"
#include "sedit.h"
#include "id3v1.h"
#include "id3v2.h"
#include "fileops.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Note: I'm devoting quite a bit of code to glue the interface from C to C++,
  and I'm not entirely happy about it. I'm a bit confused as to the reasons I
  didn't chose to write id3v2.c in C++. :(

*/

using namespace std;

typedef map<string,string> db;

/* ===================================== */

 // extra hairyness to prevent buffer overflows by re-allocating on the fly
 // overkill, but i had to do a runtime check anyway, so.

struct w_ptr {
    size_t avail;
    char*  base;

    operator char*()  { return base; }

    w_ptr(size_t len) { base = (char*) malloc(avail=len);
                        if(!base) throw bad_alloc(); }
   ~w_ptr()           { free(base); }

    char* put(char* dst, const char* ID, const void* src, size_t len);
};

char* w_ptr::put(char* dst, const char* ID, const void* src, size_t len)
{
    static size_t factor = 0x1000;      // start reallocing in 4k blocks

    if(len+10 > avail) {
        while(len+10 > factor) factor *= 2;
        int size = dst - base;
        avail    = factor;
        base     = (char*) realloc(base, size+factor);
        if(!base) throw bad_alloc();
        dst      = base + size;      // translate current pointer
    }

    avail -= (len+10);
    return (char*) ID3_put(dst,ID,src,len);
}

/* ===================================== */

 // convert C handler to a C++ exception at program startup

extern "C" int w_handler(const char*, const char*);

struct w_fail {
    w_fail()            { ID3_wfail = w_handler; }
    static string err;
    static void raise();
} w_fail_inst;

string w_fail::err;

void w_fail::raise()
{
    string emsg(err);
    err.erase();
    if(!emsg.empty())
        throw smartID3::failure(emsg);
}

extern "C" int w_handler(const char* oldn, const char* newn)
{
    if(! cpfile(oldn, newn) ) {
        string emsg(" lost, new contents still in ");
        w_fail::err = newn + emsg + oldn;
    }
    return 1;
}

/* ===================================== */

const char xlat[][5] = {
    "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
};

smartID3v2& smartID3v2::set(ID3set i, const char* m)
{
    if(i < ID3_MAX) {
        const string prefix("\0eng\0", i!=cmnt? 1 : 5);
        if(i == genre) {
            unsigned int x = atoi(m)-1;             // is portable
            if(x < ID3v1_numgenres) m = ID3v1_genre[x];
        }
        mod2.insert( db::value_type(xlat[i], prefix+m) );
        smartID3::set(i,m);                         // chain to parent
    }
    return *this;
}

template<void clean(void*)> struct voidp {          // auto-ptr like
    void* data;
    operator void*() { return data; }
    voidp(void* p)   { data = p;    }
   ~voidp()          { clean(data); }
};

bool smartID3v2::vmodify(const char* fn, const base_container& v) const
{
    if(!v2)
        return v1 && smartID3::vmodify(fn, v);

    voidp<ID3_free>
          src ( fresh ? (void*)0 : ID3_readf(fn, 0) );
    w_ptr dst ( 0x1000 );
    db    cmod( mod2 );

    char* out = (char*) ID3_put(dst,0,0,0);         // initialize

    if( src ) {                                     // update existing tags
        ID3FRAME f;
        ID3_start(f, src);

        while(ID3_frame(f)) {
            db::iterator p = cmod.find(f->ID);
            if(p == cmod.end())
                out = dst.put(out, f->ID, f->data, f->size);
            else
                if(p->second != "") {               // else: erase frames
                    string s = edit(p->second, v);
                    out = dst.put(out, f->ID, s.c_str(), s.length());
                    cmod.erase(p);
                }
        }
    }

    for(db::iterator p = cmod.begin(); p != cmod.end(); ++p) {
        if(p->second != "") {
            string s = edit(p->second, v);
            out = dst.put(out, p->first.c_str(), s.c_str(), s.length());
        }
    }

    bool res = ID3_writef(fn, dst);
    w_fail::raise();

    return res && (!v1 || smartID3::vmodify(fn, v));
}

