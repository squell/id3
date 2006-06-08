#include <vector>
#include <memory>
#include <new>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "lyrics3.h"
#include "getid3.h"
#include "getlyr3.h"

/*

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;

using tag::read::Lyrics3;
using tag::ID3field;

Lyrics3::Lyrics3(const char* fn)
: ID3(fn), lyrics_tag(lyrics3::read(fn,0))
{
}

const char Lyrics3::field_name[3][4] = { "ETT", "EAR", "EAL" };

Lyrics3::value_string Lyrics3::operator[](ID3field field) const
{
    const value_string& id3 = ID3::operator[](field);
    const char (*ptr)[4] = field_name;
    switch(field) {
    default:
        return id3;
    case tag::album:  ++ptr;
    case tag::artist: ++ptr;
    case tag::title:
        value_string lyr3 = lyrics3::find(lyrics_tag, *ptr);
        return (lyr3.substr(0, id3.length()) == id3)? lyr3 : id3;
    }
}

/* ====================================================== */

Lyrics3::array Lyrics3::listing() const
{
    throw this;
}

