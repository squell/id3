#include <cstdio>

#include <string>
#include <map>
#include <cctype>
#include "setid3v2.h"
#include "id3v2.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

typedef map<string,string>::iterator   map_ptr;
typedef map<string,string>::value_type map_el;

const char xlat[][5] = {
    "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
};

smartID3v2& smartID3v2::set(ID3set i, const char* m)
{
    if(i < ID3) {
        const string t = (i==cmnt?"\0eng\0":"\0");

        mod2.insert( map_el(xlat[i], t+m) );
        smartID3::set(i,m);                       // chain to parent
    }
    return *this;
}
 
bool smartID3v2::vmodify(const char* fn, const base_container& v)
{
    printf("[%s]\n", fn);
    for(map_ptr p = mod2.begin(); p != mod2.end(); ++p) {
        printf("%s(%s)\n", p->first.c_str(), p->second.c_str());
    }
/*
    unsigned long size;
    void* src = ID3_readf(fn, &size);
    char* dst = new char[size+1];
    char* out = dst;

    map<string,string> cmod(mod2);

    *out = 0;

    if(!fresh) {                                 // update existing tags
        ID3FRAME f;
        ID3_start(f, src);

        while(ID3_frame(f)) {
            map_ptr p = cmod.find(f->ID);
            if(p == cmod.end())
                out = (char*) ID3_put(out, f->ID, f->data, f->size);
            else {
                string s = edit(p->second, v);
                out = (char*) ID3_put(out, p->first.c_str(), s.c_str(), s.length());
                cmod.erase(p);
            }
        }
    }

    for(map_ptr p = cmod.begin(); p != cmod.end(); p++) {
        string s = edit(p->second, v);
        out = (char*) ID3_put(out, p->first.c_str(), s.c_str(), s.length());
    }

    ID3_writef(fn, dst);
    ID3_free(src);
    delete[] dst;
*/
}

