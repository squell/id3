#include <cstdlib>
#include <cstring>
#include <iterator>
#include <string>
#include "utf8.h"
#include "char_utf8.h"

/*

  character conversion (<-> utf-8 encoding)

  copyright (c) 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

namespace charset {

    template<> conv<>::data conv<utf8>::decode(const char* s, size_t len)
    {
        conv<>::data build;
        build.reserve(len);
        ::utf8::decode(s, s+len, std::back_inserter(build));
        return build;
    }

    template<> std::string conv<utf8>::encode(const void* p, size_t len)
    {
        const wchar_t* s = (wchar_t*)p;
        std::string build;
        build.reserve(len);
        ::utf8::encode(s, s+len, std::back_inserter(build));
        return build;
    }

} // end of namespace

