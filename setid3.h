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
#include <utility>
#include "set_base.h"

struct ID3v1;                                 // avoid a header dependency

namespace tag {

    namespace write {

        class ID3 : public handler, public reader {
        public:
            bool      vmodify(const char*, const function&) const;
            metadata* read(const char*) const;

          // standard set
            ID3() : update(), cleared(), generate(), null_tag() { }
           ~ID3();

            ID3& set(ID3field i, std::string m)
            { if(i < FIELD_MAX) update[i] = m; return *this; }

            ID3& rewrite(bool t = true)
            { cleared = t;  return *this; }

            ID3& create(bool t = true)
            { generate = t; return *this; }

          // extended
            bool from(const char* fn);

        private:
            struct nullable : private std::pair<std::string, bool> {
                struct null;
                void operator=(const null*)           { first.erase(), second = 0; }
                void operator=(std::string p)         { first.swap(p), second = 1; }
                operator const std::string*() const   { return second? &first : 0; }
                const std::string* operator->() const { return *this; }
            };
            nullable update[FIELD_MAX]; // modification data
            bool cleared;               // should vmodify clear existing tag?
            bool generate;              // don't *add* new tags to files?
            const ID3v1* null_tag;      // use as base tag
        };

    }
}

#endif

