#include <string>
#include <map>
#include <cctype>
#include <cstdlib>
#include "setid3v2.h"
#include "id3v2.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

/* ===================================== */

 // extra hairyness to prevent buffer overflows by re-allocating on the fly
 // this may seem like overkill, but i had to do a runtime check anyway, so.

struct w_ptr {
    unsigned long avail;
    char*         data;
};

 // employs a 'hint' system. if 4kb extra isn't enough, it'll try 8kb,
 // then 16kb, etc. next time, it'll immediately try 16kb again, etc.

char* put(char* dst, const char* ID, const void* src, size_t len, w_ptr& base)
{
    static unsigned long factor = 0x1000;  // start reallocing in 4k blocks

    if(len+10 > base.avail) {
        while(len+10 > factor) factor *= 2;
        int size   = dst-base.data;
        base.avail = factor;
        base.data  = (char*) realloc(base.data, size+factor);
        dst        = base.data + size;     // translate current pointer
    }

    base.avail -= (len+10);
    return (char*) ID3_put(dst,ID,src,len);
}

/* ===================================== */

const char xlat[][5] = {
    "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
};

smartID3v2& smartID3v2::set(ID3set i, const char* m)
{
    if(i < ID3) {
        const string t("\0eng\0", i!=cmnt? 1 : 5);

        mod2.insert( db::value_type(xlat[i], t+m) );
        smartID3::set(i,m);                         // chain to parent
    }
    return *this;
}

bool smartID3v2::vmodify(const char* fn, const base_container& v) const
{
    if(v1 && !v2)
        return smartID3::vmodify(fn, v);

    w_ptr dst;
    void* src = ID3_readf(fn, &dst.avail);
                dst.avail = dst.avail+0x1000;
    char* out = dst.data  = (char*) malloc(dst.avail);

    db cmod(mod2);

    ID3_put(out, 0, 0, 0);                          // initialize

    if(!fresh) {                                    // update existing tags
        ID3FRAME f;
        ID3_start(f, src);

        while(ID3_frame(f)) {
            db::iterator p = cmod.find(f->ID);
            if(p == cmod.end())
                out = put(out, f->ID, f->data, f->size, dst);
            else
                if(p->second != "") {               // else: erase frames
                    string s = edit(p->second, v);
                    out = put(out, f->ID, s.c_str(), s.length(), dst);
                    cmod.erase(p);
                }
        }
    }

    for(db::iterator p = cmod.begin(); p != cmod.end(); ++p) {
        if(p->second != "") {
            string s = edit(p->second, v);
            out = put(out, p->first.c_str(), s.c_str(), s.length(), dst);
        }
    }

    bool res = ID3_writef(fn, dst.data);
    ID3_free(src);
    free(dst.data);

    if(v1 && res)
        return smartID3::vmodify(fn, v);
    else
        return res;
}

