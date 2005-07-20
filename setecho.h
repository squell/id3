/*

  set_tag::echo applicative class

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The set_tag::echo class doesn't set anything, but reports back info.

*/

#ifndef __ZF_SETQUERY
#define __ZF_SETQUERY

#include <string>
#include "set_base.h"

namespace set_tag {

class echo : public handler {
    std::string fmt;
public:
    echo& format(std::string s)
    { fmt=s; return *this; }

    virtual bool vmodify(const char*, const subst&) const;
    virtual void log(const cvtstring&) const;

  // standard set - dummies

    echo& set(ID3field, std::string)
    { return *this; }

    echo& clear()
    { return *this; }
};

}

#endif

