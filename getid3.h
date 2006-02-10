/*

  tag::read::ID3

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The read::ID3 class implements the reader interface for ID3 tags

*/

#ifndef __ZF_GETID3
#define __ZF_GETID3

#include <string>
#include "set_base.h"
#include "id3v1.h"

namespace tag {
    namespace read {

        class ID3 : public metadata {
        public:
            ID3v1 const tag;

            typedef metadata::factory<ID3> factory;
            explicit ID3(const char* fn);
            value_string operator[](ID3field field) const;
            array listing() const;
            operator bool() const { return tag.TAG[0]; }
        };

    }
}

#endif

