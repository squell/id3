/*

  set_tag::filename applicative class

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The set_tag::filename class implements file renaming (as a pseudo-tag)

  Example:

  int main(int argc, char* argv[])
  {
      set_tag::fname()
      .rename("%2")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETFNAME
#define __ZF_SETFNAME

#include <string>
#include "set_base.h"

namespace set_tag {

class filename : public handler {
    std::string ftemplate;
public:
    filename& rename(std::string fname)
    { ftemplate=fname; return *this; }

    virtual bool vmodify(const char*, const subst&) const;

  // standard set - dummies

    filename& set(ID3field, std::string)
    { return *this; }

    filename& clear()
    { return *this; }
};

}

#endif

