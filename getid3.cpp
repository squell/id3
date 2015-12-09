#include <vector>
#include <cstdio>
#include <cstring>
#include "getid3.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;

using tag::read::ID3;
using tag::ID3field;

namespace {
    inline ID3v1 readtag(const char *fn)
    {
        struct ID3v1 tag = { { 0, } };
        if( FILE* f = fopen(fn, "rb") ) {
            fseek(f, -128, SEEK_END)  == 0    &&
            fread(&tag, 1, 128, f)    == 128  &&
            memcmp(tag.TAG, "TAG", 3) == 0    || (tag.TAG[0] = 0);
            fclose(f);
        }
        return tag;
    }
}

ID3::ID3(const char* fn) : tag(readtag(fn))
{
}

ID3::value_string ID3::operator[](ID3field field) const
{
    char buf[31]     = { 0, };         // enough to hold largest ID3 field+1

    if(*tag.TAG)
        switch( field ) {
        case title:
            strncpy(buf, tag.title,  sizeof tag.title);
            break;
        case artist:
            strncpy(buf, tag.artist, sizeof tag.artist);
            break;
        case album:
            strncpy(buf, tag.album,  sizeof tag.album);
            break;
        case year:
            strncpy(buf, tag.year,   sizeof tag.year);
            break;
        case cmnt:
            strncpy(buf, tag.cmnt,   sizeof tag.cmnt + (tag.zero?2:0));
            break;
        case track:
            if(tag.zero == 0 && tag.track != 0)
                sprintf(buf, "%u", tag.track & 0xFF);
            break;
        case genre:
            if(tag.genre < ID3v1_numgenres)
                return conv<latin1>(ID3v1_genre[tag.genre]);
            break;
        case FIELD_MAX:
            char ver[] = "ID3v1._";
            ver[6] = '0'+!tag.zero;
            return ver;
        }
    return conv<latin1>(buf);
}

/* ====================================================== */

namespace {                     // don't display in the order of of ID3field
    using namespace tag;

    const char* desc[] = {
        "Title", "Artist", "Album", "Track", "Year", "Genre", "Comment"
    };

    const ID3field tab[] = {
        title, artist, album, track, year, genre, cmnt
    };
}

ID3::array ID3::listing() const
{
    array vec;
    vec.push_back( array::value_type("ID3", tag.zero? "1.0" : "1.1") );
    for(int x = 0; x < FIELD_MAX; ++x) {
        conv<> s = operator[](tab[x]);
        if(!s.empty())
            vec.push_back( array::value_type(desc[x], s) );
    }
    return vec;
}

