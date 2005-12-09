/*

  set_tag::rename applicative class

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

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

class rename : public group {
    std::string ftemplate;
public:
    rename& filename(std::string fname)
    { ftemplate=fname; return *this; }

    virtual bool vmodify(const char*, const subst&) const;
};

}

#endif

