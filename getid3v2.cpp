#include <vector>
#include <cstdio>
#include "id3v2.h"
#include "getid3v2.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;
using namespace charset;

using set_tag::read::ID3v2;
using set_tag::ID3field;

static bool getframe(const void*, ID3FRAME, int n, const char*);
static conv<> unbinarize(ID3FRAME);

ID3v2::ID3v2(const char* fn) : tag(ID3_readf(fn,0))
{
}

ID3v2::~ID3v2()
{
    ID3_free(tag);
}

ID3v2::value_string ID3v2::operator[](ID3field field) const
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
    bool ok = false;
    if(tag && field < FIELDS) {
        const bool v = ID3_start(f, tag) > 2;
        ok = getframe(tag, f, 3+v, fieldtag[field][v]);
    }
    return ok? unbinarize(f) : conv<>();
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

static conv<> unbinarize(ID3FRAME f)
{
    const string field  = f->ID;
    const char*  p      = f->data + 1;

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
            return conv<latin1>(p, f->size-hdrsiz);
        default:
            return conv<latin1>("<unsupported encoding>");
        };
    } else if(ID3v2::is_url(field)) {
        return conv<latin1>(f->data, f->size);
    } else {
        return conv<>();
    }
}

