/*

  set_tag::read::ID3v2

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The read::ID3v2 class implements the reader interface for ID3v2 tags

*/

#ifndef __ZF_GETID3V2
#define __ZF_GETID3V2

#include <string>
#include "set_base.h"
#include "charconv.h"

namespace set_tag {
    namespace read {

        class ID3v2 : public reader {
            void* tag;
        public:
            ID3v2(const char* fn);
           ~ID3v2();
            cvtstring operator[](ID3field field) const;
            array listing() const;
        };

    }
}

#endif

