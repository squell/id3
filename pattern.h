/*

  specify tag fields using pattern matching (extension)

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  Usage:

  Converts a string like "%a - %t.mp3" to "* - *.mp3" and sets field in a
  tag accordingly to %1, %2, etc. Can be tested as a truth value to ascertain
  whether fields have been set by this operation.

  Example:

  ID3 tag;
  mass_tag(tag).glob( pattern(tag, "%n - %t.mp3") );

*/

#ifndef __ZF_PATTERN_HPP
#define __ZF_PATTERN_HPP

#include <string>
#include "set_base.h"

class pattern {
    std::string str;
    bool ok;
public:
    pattern(set_tag::handler& tag, std::string mask);
    operator bool() const           { return ok; }
    const std::string& mask() const { return str; }
};

#endif

