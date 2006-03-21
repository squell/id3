/*

  tag::write::Lyrics3 applicative class

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The write::Lyrics3 class implements the interface for Lyrics3 tags

*/

#ifndef __ZF_SETLYRICS3
#define __ZF_SETLYRICS3

#include <map>
#include <string>
#include "set_base.h"

namespace tag {
    namespace write {

        class Lyrics3 : public handler, public reader {
        public:
            bool      vmodify(const char*, const function&) const;
            metadata* read(const char*) const;

          // standard set
            Lyrics3() : mod(), null_tag(), fresh(), gen() { }

            Lyrics3& set(ID3field i, std::string m);
            Lyrics3& rewrite(bool t = true)
            { fresh = t; return *this; }
            Lyrics3& create(bool t = true)
            { gen   = t; return *this; }

          // extended
            bool set(std::string field, std::string s);
            bool rm(std::string field);
            bool from(const char* fn);

        private:
            std::map<std::string,std::string> mod;
            std::string null_tag;
            bool fresh, gen;
        };

    }
}

#endif

