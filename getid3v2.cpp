#include <vector>
#include <cstdio>
#include "getid3v2.h"
#include "id3v2.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

using set_tag::read::ID3v2;
using set_tag::ID3field;

static bool getframe(void*, ID3FRAME, int n, const char*);
static cvtstring unbinarize(ID3FRAME); 

ID3v2::ID3v2(const char* fn) : tag(ID3_readf(fn,0))
{
}

ID3v2::~ID3v2()
{
    ID3_free(tag);
}

cvtstring ID3v2::operator[](ID3field field) const
{
    const char* const fieldtag[FIELDS][2] = {          // ID3field order!
        { "TT2"  "TT3"  "TOF",
          "TIT2" "TIT3" "TOFN" "TRSN" },
        { "TP1"  "TCM"  "TP2"  "TP3"  "TP4"  "TXT"  "TOA"  "TOL"  "TPB",
          "TPE1" "TCOM" "TPE2" "TPE3" "TPE4" "TEXT" "TOPE" "TOLY" "TPUB" "TRSO" },
        { "TAL"  "TOT",
          "TALB" "TOAL" },
        { "TYE"  "TOR",
          "TYER" "TORY" },
        { "COM"  "TCR",
          "COMM" "TCOP" "USER" },
        { "TRK"  "TPA",
          "TRCK" "TPOS" },
        { "TCO"  "TT1",
          "TCON" "TIT1" }
    };

    ID3FRAME f;
    bool v  = ID3_start(f, tag) > 2;
    bool ok = false;
    if(tag && field < FIELDS)
        ok = getframe(tag, f, 3+v, fieldtag[field][v]);
    return ok? unbinarize(f) : cvtstring();
}

/* ====================================================== */

ID3v2::array ID3v2::listing() const
{
    ID3FRAME f;

    array vec;
    if(tag) {
        ID3VER version = ID3_start(f, tag);
        char   vstr[2] = { version+'0' };
        vec.push_back( array::value_type("ID3v2", vstr) );
        while(ID3_frame(f)) {
            vec.push_back( array::value_type(f->ID, unbinarize(f)) );
        }
    }
    return vec;
}

/* ====================================================== */

static bool getframe(void* tag, ID3FRAME f, int n, const char* field)
{
    for( ; *field != '\0'; field+=n) {
        ID3_start(f, tag);
        while(ID3_frame(f))
            if(strncmp(field, f->ID, n) == 0) {
                if(f->ID[0] == 'T') {
                    if(f->size > 1) return 1;
                } else
                    return 1;
            }
    }
    return 0;
}

static cvtstring unbinarize(ID3FRAME f)
{
    const char   nul[2] = { 0 };
    const string field  = f->ID;
    const char*  p      = f->data + 1;

    if(ID3v2::is_counter(field)) {
        char buf[12];                          // enough for 32bits
        unsigned long t = 0;
        for(size_t n = 0; n < f->size; ++n)
            t = t << 8 | (f->data[n] & 0xFF);
        sprintf(buf, "%lu", t & 0xFFFFFFFFul);
        return cvtstring::latin1(buf);
    }

    if(ID3v2::has_lang(field))
        p += 3;                                // skip-ignore language field
    if(ID3v2::has_desc(field)) {
        const int   skip = !!*f->data;
        const char* lim  = f->data + f->size - skip;  // safety
        const char* q;
        for(q = p; q < lim && (*q || q[-skip]); )
            ++(q += skip);                     // find null (grmbl)
        if(q++ == lim)
            return cvtstring();                // error
        p = q;
    }

    size_t hdrsiz = p - f->data;
    if(hdrsiz > 1 || ID3v2::is_text(field)) {
        switch(*f->data) {
        case  0:
            return cvtstring::latin1(string(p, f->size-hdrsiz));
        default:
            return cvtstring::latin1("<unsupported encoding>");
        };
    } else if(ID3v2::is_url(field)) {
        return cvtstring::latin1(string(f->data, f->size));
    } else {
        return cvtstring();
    }
}

/*

 Personal note.

 I admire M. Nilsson for being brave enough to put his name above documents
 such as the various ID3v2 standards.

*/

