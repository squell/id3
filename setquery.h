/*

  tag::write::query applicative class

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The write::query class doesn't set anything, but reports back info.

*/

#ifndef __ZF_SETQUERY
#define __ZF_SETQUERY

#include <string>
#include "set_base.h"

namespace tag {
    namespace write {

        class query : public writer {
            std::string fmt;
        public:
            query& print(std::string s)
            { fmt = s; return *this; }

            virtual bool vmodify(const char*, const function&) const;
            virtual void log(const char*) const;
        };

    }
}

#endif

