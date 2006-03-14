/*

  tag::write::file applicative class

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The write::file class implements 'meta' operations for a file
  (tagging and renaming)

  Example:

  int main(int argc, char* argv[])
  {
      tag::write::file()
      .filename("%2")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETFNAME
#define __ZF_SETFNAME

#include <string>
#include "setgroup.h"

namespace tag {
    namespace write {

        class file : public combined<handler> {
            std::string m_template;
            bool m_preserve;
        public:
            file() : m_preserve(0) { }

            file& rename(std::string fname)
            { m_template=fname; return *this; }

            file& touch(bool t = true)
            { m_preserve=!t; return *this; }

            virtual bool vmodify(const char*, const function&) const;
        };
    }

}

#endif

