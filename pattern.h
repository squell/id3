/*

  specify tag fields using pattern matching (extension)

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

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

class pattern : public std::string {
    std::string str;
    unsigned num;
public:
    pattern(tag::handler& tag, std::string mask);
    unsigned vars() const { return num; }
};

#endif

