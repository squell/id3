/*

  tag::write::echo applicative class

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The write::echo class doesn't set anything, but reports back info.

*/

#ifndef __ZF_SETQUERY
#define __ZF_SETQUERY

#include <string>
#include "set_base.h"

namespace tag {
    namespace write {

        class echo : public handler {
            const std::string fmt;
        public:
            echo(std::string s) : fmt(s) { }

            virtual bool vmodify(const char*, const function&) const;
            virtual void log(const char*) const;

          // standard set - dummies

            echo& set(ID3field, std::string)
            { return *this; }

            echo& rewrite(bool)
            { return *this; }
        };

    }
}

#endif

