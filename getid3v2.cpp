#include <vector>
#include <cstdio>
#include <cstring>
#include "char_ucs.h"
#include "char_utf8.h"
#include "id3v2.h"
#include "getid3v2.h"

/*

  copyright (c) 2004, 2005, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;

using tag::read::ID3v2;
using tag::ID3field;

static bool getframe(ID3FRAME f, const char* field, size_t n);

// setid3v2 also needs the following function; but the outside world doesn't
namespace tag {
    extern ID3v2::value_string unbinarize(ID3FRAME, charset::conv<>& descriptor);
}

ID3v2::ID3v2(const char* fn) : tag(ID3_readf(fn,0))
{
}

ID3v2::ID3v2(const void* id3v2_data) : tag(id3v2_data)
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
    if(tag && field < FIELD_MAX) {
        const bool  version = ID3_start(f, tag) > 2;
        const char*  id_str = fieldtag[field][version];
        const size_t id_len = 3+version;

        // return the first ID of the above list that produces a clear match
        for( ; *id_str != '\0'; id_str += id_len) {
            ID3_start(f, tag);
            while( getframe(f, id_str, id_len) ) {
                charset::conv<local> desc;
                value_string val = unbinarize(f, desc);
                if(desc.length() <= 5) // allow for "::lan"; which works because
                    return val;        // in fieldtag: has_desc() implies has_lang()
            }
        }
    } else if(tag && field == FIELD_MAX) {
        ID3_start(f, tag);
        char buf[] = "ID3v2._";
        buf[6] = f->_rev+'2';
        return buf;
    }
    return value_string();
}

/* ====================================================== */

ID3v2::array ID3v2::listing() const
{
    ID3FRAME f;

    array vec;
    if(tag) {
        ID3VER version = ID3_start(f, tag);
        char   vstr[4] = { char(version+'0'), '.', '0', };
        vec.push_back( array::value_type("ID3v2", vstr) );
        while(ID3_frame(f)) {
            charset::conv<local> desc;
            value_string val = unbinarize(f, desc);
            vec.push_back( array::value_type(f->ID+string(desc), val) );
        }
    }
    return vec;
}

/* ====================================================== */

static bool getframe(ID3FRAME f, const char* field, size_t n)
{
    while(ID3_frame(f))
        if(strncmp(field, f->ID, n) == 0) {
            if(f->ID[0] == 'T') {
                if(f->size > 1) return 1;
            } else
                return 1;
        }
    return 0;
}

  // safely find a terminator in multi-string frames

static const char *membrk0(const char* buf, size_t size, bool wide)
{
    const char* const end = buf + size - wide;
    const int step = 1+wide;
    for( ; buf < end; buf += step) {
        if(!buf[0] && !buf[wide])
            return buf;
    }
    return 0;
}

extern ID3v2::value_string tag::unbinarize(ID3FRAME f, charset::conv<>& descriptor)
{
    typedef conv<latin1> cs;

    const string field  = f->ID;
    const char*  p      = f->data + 1;
    bool wide           = *f->data == 1 || *f->data == 2;

    if(f->packed || f->encrypted || f->grouped)
        return ID3v2::value_string(cs("<compressed or encrypted>"),0);

    if(ID3v2::is_counter(field)) {
        const char* data = f->data;
        if(ID3v2::has_desc(field)) {
            if(!(data = membrk0(data, f->size, 0))) return conv<>();
            descriptor = conv<charset::latin1>(":") += conv<charset::latin1>(f->data, data - f->data);
            data += 2;                         // data[-1] points to rating
        }
        char buf[12];                          // enough for 32bits
        unsigned long t = 0;
        for(size_t n = 0; n < f->size - (data - f->data); ++n)
            t = t << 8 | (data[n] & 0xFF);
        if(ID3v2::has_desc(field)) {
            sprintf(buf, "%u:%lu", data[-1]&0xFFu, t & 0xFFFFFFFFul);
        } else
            sprintf(buf, "%lu", t & 0xFFFFFFFFul);
        return conv<latin1>(buf);
    }

    const char* lang = 0;
    if(ID3v2::has_lang(field)) {
        lang = p;
        if(memchr(lang,'\0',3))                // some programs use null-bytes
            lang = "xxx";
        p += 3;                                // skip-ignore language field
    }
    if(ID3v2::has_desc(field)) {
        const char* q = membrk0(p, f->size - (p - f->data), wide);
        if(!q) return conv<>();                // malformed frame
        descriptor = conv<charset::latin1>(":");
        switch(*f->data) {
            case 0: descriptor += conv<charset::latin1> (p, q-p); break;
            case 1: descriptor += conv<charset::utf16>  (p, q-p); break;
            case 2: descriptor += conv<charset::utf16be>(p, q-p); break;
            case 3: descriptor += conv<charset::utf8>   (p, q-p); break;
        }
        p = q+1+wide;
    } else if(lang) {
        descriptor += ':';                     // only used by USER; enfore consistency in notation
    }
    if(lang)
        (descriptor += ':') += conv<charset::latin1>(lang,3);

    size_t hdrsiz = p - f->data;
    if(hdrsiz > 1 || ID3v2::is_text(field)) {
        size_t txtsiz = f->size - hdrsiz;
        char encoding = *f->data;
        if(ID3v2::is_url(field)) {
            wide = encoding = 0;               // the WXXX frame is so weird
        }
        if(const char *q = membrk0(p, txtsiz, wide)) {
            txtsiz = q - p;                    // useless null-terminator
        }
        switch(encoding) {
        case  0:
            return conv<latin1> (p, txtsiz);
        case  1:
            return conv<utf16>  (p, txtsiz);
        case  2:
            return conv<utf16be>(p, txtsiz);
        case  3:
            return conv<utf8>   (p, txtsiz);
        default:
            return ID3v2::value_string(cs("<unsupported encoding>"),0);
        };
    } else if(ID3v2::is_url(field)) {
        return conv<latin1>(f->data, f->size);
    } else {
        return ID3v2::value_string(cs("<binary data>"), 0);
    }
}

