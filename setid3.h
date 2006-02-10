/*

  tag::write::ID3 applicative class

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The write::ID3 class implements the interface for ID3 tags

  Example:

  int main(int argc, char* argv[])
  {
      tag::write::ID3()
      .set(artist, "%2")
      .set(title,  "%3")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETID3
#define __ZF_SETID3

#include <string>
#include "set_base.h"

struct ID3v1;                                 // avoid a header dependency

namespace tag {
    namespace write {

        class ID3 : public handler, public reader, private handler::body {
        public:
            bool      vmodify(const char*, const function&) const;
            metadata* read(const char*) const;

          // standard set
            ID3() : null_tag() { }
           ~ID3();

            ID3& set(ID3field i, std::string m)
            { handler::body::set(i, m); return *this; }

            ID3& clear(bool t = true)
            { handler::body::clear(t);  return *this; }

          // extended
            bool from(const char* fn);

        private:
            const ID3v1* null_tag;
        };

    }
}

#endif

