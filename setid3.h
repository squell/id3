/*

  set_tag::ID3 applicative class

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  Usage:

  The set_tag::ID3 class implements the single_tag interface for ID3 tags

  Example:

  int main(int argc, char* argv[])
  {
      set_tag::ID3()
      .set(artist, "%2")
      .set(title,  "%3")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETID3
#define __ZF_SETID3

#include <string>
#include "set_base.h"

namespace set_tag {

class ID3 : public handler, public provider, private handler::body {
public:
    bool    vmodify(const char*, const function&) const;
    reader* read(const char*) const;

  // standard set
    ID3() : null_tag() { }
   ~ID3();

    ID3& set(ID3field i, std::string m)     // set ID3 field i to value m
    { handler::body::set(i, m); return *this; }

    ID3& clear(bool t = true)
    { handler::body::clear(t);  return *this; }

  // extended
    bool from(const char* fn);

private:
    const void* null_tag;                   // avoid a header dependency
};

}

#endif

