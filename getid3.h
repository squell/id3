/*

  set_tag::read::ID3

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The read::ID3 class implements the reader interface for ID3 tags

*/

#ifndef __ZF_GETID3
#define __ZF_GETID3

#include <string>
#include "set_base.h"
#include "charconv.h"
#include "id3v1.h"

namespace set_tag {
    namespace read {

        class ID3 : public reader {
            ID3v1 tag;
        public:
            ID3(const char* fn);
            cvtstring operator[](ID3field field) const;
            array listing() const;
        };

    }
}

#endif

