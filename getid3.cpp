#include <string>
#include <vector>
#include <cstdio>
#include "getid3.h"
#include "charconv.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

using set_tag::read::ID3;
using set_tag::ID3field;

ID3::ID3(const char* fn) : tag()
{
    if( FILE* f = fopen(fn, "rb") ) {
        fseek(f, -128, SEEK_END);
        fread(&tag, 1, 128, f);
        if( ferror(f) || memcmp(tag.TAG, "TAG", 3) != 0 ) {
            tag.TAG[0] = 0;
        }
        fclose(f);
    }
}

cvtstring ID3::operator[](ID3field field) const
{
    const char fmt[] = "%.*s";
    char buf[31]     = { 0, };         // enough to hold largest ID3 field+1

    if(*tag.TAG)
        switch( field ) {
        case title:
            sprintf(buf, fmt, sizeof tag.title,  tag.title);
            break;
        case artist:
            sprintf(buf, fmt, sizeof tag.artist, tag.artist);
            break;
        case album:
            sprintf(buf, fmt, sizeof tag.album,  tag.album);
            break;
        case year:
            sprintf(buf, fmt, sizeof tag.year,   tag.year);
            break;
        case cmnt:
            sprintf(buf, fmt, sizeof tag.cmnt + (tag.zero?2:0), tag.cmnt);
            break;
        case track:
            if(tag.zero == 0 && tag.track != 0)
                sprintf(buf, "%u", tag.track);
            break;
        case genre:
            if(tag.genre < ID3v1_numgenres)
                return ID3v1_genre[tag.genre];
        }
    return cvtstring::latin1(buf);
}

/* ====================================================== */

namespace {
    using namespace set_tag;

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
    for(int x = 0; x < FIELDS; ++x) {
        cvtstring s = operator[](tab[x]);
        if(!s.empty())
            vec.push_back( array::value_type(desc[x], s) );
    }
    return vec;
}

