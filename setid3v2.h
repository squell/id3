/*

  tag::write::ID3v2 applicative class

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  Usage:

  The write::ID3v2 class implements the single_tag interface for ID3 tags

  Limitation/Flaws:

  ID3v2 knows only the frame types classified by read::ID3v2.

  .vmodify() doesn't know about frames having multiple instances.

  a. - when updating a frame type which has multiple instances, the first one
       will be replaced and the rest will be left unaltered.

  b. - ID3v2 will never add multiple instances of the same frame type.

  c. - deleting a frame using .rm() deletes *ALL* frames of a specific type.

  (a) requires an extra distinguisher function.
  (b) is not a real problem, but it could be solved by porting to multimap
  (c) is what users would expect, therefore intended behaviour

*/

#ifndef __ZF_SETID3V2
#define __ZF_SETID3V2

#include <string>
#include <map>

#include "set_base.h"

namespace tag {
    namespace write {

        class ID3v2 : public handler, public reader {
        public:
            ID3v2() : null_tag(), mod(), resize(), fresh()
            { }
           ~ID3v2();

            ID3v2& set(ID3field i, std::string m);      // set standard field
            ID3v2& reserve(size_t);                     // set suggested size
            ID3v2& clear(bool t = true)                 // erase previous tag
            { fresh = t; return *this; }

            bool      vmodify(const char*, const function&) const;
            metadata* read(const char*) const;

          // extended set

            bool set(std::string field, std::string s); // set ID3v2 frame
            bool rm(std::string field);                 // remove ID3v2 frame
            bool from(const char*);                     // specify copy tag

        private:
            const void* null_tag;
            std::map<std::string,std::string> mod;
            size_t resize;
            bool fresh;
        };

    }
}

#endif

