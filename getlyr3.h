/*

  tag::read::ID3_lyrics

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The read::ID3_lyrics class implements the reader interface for Lyrics3 tags.

  Notes:

  As Lyrics3 is an strange extension of ID3v1, it falls back to those tags
  for information not present in the Lyrics3 tag.

*/

#ifndef __ZF_GETLYRICS3
#define __ZF_GETLYRICS3

#include <string>
#include "lyrics3.h"
#include "getid3.h"

namespace tag {
    namespace read {

        class Lyrics3 : public ID3 {
        public:
            lyrics3::info const lyrics_tag;

            typedef metadata::factory<Lyrics3> factory;
            explicit Lyrics3(const char* fn);
            value_string operator[](ID3field field) const;
            array listing() const;
            operator bool() const { return lyrics_tag.size(); }

            static const char field_name[3][4];
        };

        typedef Lyrics3 ID3_with_lyrics;
    }
}

#endif

