/*

  set_tag::filename applicative class

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The set_tag::filename class implements file renaming (pseudo-tag). It has
  built-in chaining so it can be combined with tags without harm.

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
    handler* chain;
public:
    filename() : chain(0)                { }
    filename(handler& tag) : chain(&tag) { }

    filename& rename(std::string fname)
    { ftemplate=fname; return *this; }

    virtual bool vmodify(const char*, const subst&) const;

  // standard set - pass or do nothing

    filename& set(ID3field fld, std::string data)
    { if(chain) chain->set(fld, data); return *this; }
    filename& clear()
    { if(chain) chain->clear();        return *this; }
};

}

#endif

