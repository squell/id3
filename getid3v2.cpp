#include <vector>
#include <cstdio>
#include <cstring>
#include "char_ucs.h"
#include "id3v2.h"
#include "getid3v2.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;

using tag::read::ID3v2;
using tag::ID3field;

static bool getframe(const void*, ID3FRAME, int n, const char*);
static ID3v2::value_string unbinarize(ID3FRAME);

ID3v2::ID3v2(const char* fn) : tag(ID3_readf(fn,0))
{
}

ID3v2::~ID3v2()
{
    ID3_free(tag);
}

ID3v2::value_string ID3v2::operator[](ID3field field) const
{
    const char* const fieldtag[FIELD_MAX][2] = {       // ID3field order!
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
    bool ok = false;
    if(tag && field < FIELD_MAX) {
        const bool v = ID3_start(f, tag) > 2;
        ok = getframe(tag, f, 3+v, fieldtag[field][v]);
    }
    return ok? unbinarize(f) : value_string();
}

/* ====================================================== */

ID3v2::array ID3v2::listing() const
{
    ID3FRAME f;

    array vec;
    if(tag) {
        ID3VER version = ID3_start(f, tag);
        char   vstr[2] = { char(version+'0'), };
        vec.push_back( array::value_type("ID3v2", vstr) );
        while(ID3_frame(f)) {
            vec.push_back( array::value_type(f->ID, unbinarize(f)) );
        }
    }
    return vec;
}

/* ====================================================== */

static bool getframe(const void* tag, ID3FRAME f, int n, const char* field)
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

static ID3v2::value_string unbinarize(ID3FRAME f)
{
    typedef conv<latin1> cs;

    const string field  = f->ID;
    const char*  p      = f->data + 1;

    if(f->packed || f->encrypted || f->grouped)
        return ID3v2::value_string(cs("<compressed or encrypted>"),0);

    if(ID3v2::is_counter(field)) {
        char buf[12];                          // enough for 32bits
        unsigned long t = 0;
        for(size_t n = 0; n < f->size; ++n)
            t = t << 8 | (f->data[n] & 0xFF);
        sprintf(buf, "%lu", t & 0xFFFFFFFFul);
        return conv<latin1>(buf);
    }

    if(ID3v2::has_lang(field))
        p += 3;                                // skip-ignore language field
    if(ID3v2::has_desc(field)) {
        const int   skip = !!*f->data;
        const char* lim  = f->data + f->size - skip;  // safety
        const char* q;
        for(q = p; q < lim && (*q || q[-skip]); )
            q += 1 + skip;                     // find null (grmbl)
        if(q++ == lim)
            return conv<>();                   // error
        p = q;
    }

    size_t hdrsiz = p - f->data;
    if(hdrsiz > 1 || ID3v2::is_text(field)) {
        switch(*f->data) {
        case  0:
            return conv<latin1> (p, f->size-hdrsiz);
        case  1:
            return conv<utf16>  (p, f->size-hdrsiz);
        case  2:
            return conv<utf16be>(p, f->size-hdrsiz);
        default:
            return ID3v2::value_string(cs("<unsupported encoding>"),0);
        };
    } else if(ID3v2::is_url(field)) {
        return conv<latin1>(f->data, f->size);
    } else {
        return conv<>();
    }
}

