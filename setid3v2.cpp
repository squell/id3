#include "setid3v2.h"
#include "id3v2.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

/*
const char smartID3v2::xlat[][5] = {
    "TIT2", "TPE1", "TALB", "TYER", "COMM", "TCON", "TRCK"
};

smartID3v2& smartID3v2::set(ID3set i, const char* m);
{
    if(i<ID3) {
        mod2[xlat[i]] = m;
        smartID3::set(i, m);                      // update ID3v1 info also
    }
    return *this;
}
*/

char* smartID3v2::put(char* out, const map_ptr& p, const base_container& v)
{
    string s = edit(p->second, v);
    int    n = s.length();
    char*  a = new char[n+1];

    a[ s.copy(a+1,n) ] = 0;                      // copy to temp buffer

    out = (char*) ID3_put(out, p->first.c_str(), a, s.length()+1);

    delete[] a;
    return out;
}

bool smartID3v2::vmodify(const char* fn, const base_container& v)
{
    unsigned long size;
    void* src = ID3_readf("test2.mp3", &size);
    char* dst = new char[size+1];
    char* out = dst;

    map<string,string> mod(mod2);

    *out = 0;

    if(!fresh) {                                 // update existing tags
        ID3FRAME f;
        ID3_start(f, src);

        while(ID3_frame(f)) {
            map_ptr p = mod.find(f->ID);
            if(p == mod.end())
                out = (char*) ID3_put(out, f->ID, f->data, f->size);
            else {
                out = put(out, p, v);
                mod.erase(p);
            }
        }
    }

    for(map_ptr p = mod.begin(); p != mod.end(); p++)
        out = put(out, p, v);

    ID3_writef(fn, dst);
    ID3_free(src);
    delete[] dst;
}

