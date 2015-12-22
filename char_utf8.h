/*

  character conversion (<-> utf-8 encoding)

  copyright (c) 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage: see charconv.h

  Uses partial specialization to encode various characteristics.

*/

#ifndef __ZF_CHARCONV_UTF8_HPP
#define __ZF_CHARCONV_UTF8_HPP

#include "charconv.h"

namespace charset {

    struct utf8;

    template<> std::wstring conv<utf8>::decode(const char*, std::size_t);
    template<> std::string  conv<utf8>::encode(const void*, std::size_t);

}

#endif

