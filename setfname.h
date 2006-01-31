/*

  set_tag::rename applicative class

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  Usage:

  The set_tag::filename class implements file renaming (pseudo-tag) or
  renaming and tagging

  Example:

  int main(int argc, char* argv[])
  {
      set_tag::rename()
      .filename("%2")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETFNAME
#define __ZF_SETFNAME

#include <string>
#include "set_base.h"

namespace set_tag {

class file : public group {
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

#endif

