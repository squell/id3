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

#include "set_base.h"

namespace set_tag {

class filename : public single_tag {
    const char* ftemplate;
public:
    filename(bool f = true) : single_tag(f) { }

    bool rename(const char* fname)
    { return ftemplate=(*fname? fname : 0); }

    virtual bool vmodify(const char*, const subst&) const;

  // standard set - dummies

    filename& set(ID3field, const char*)
    { return *this; }

    filename& clear()
    { return *this; }
};

}

#endif

