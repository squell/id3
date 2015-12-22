/*

  character conversion (user locale <-> latin1)

  copyright (c) 2005, 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The cvtstring class encapsulates an encoding-neutral string. Uses templates
  heavily for avoiding compile dependencies and using the C++ type system
  itself for specifying conversions. A limited string interface is provided
  for constructing, but not editing strings.

  Example:

  std::puts( conv<latin1>("ISO 8859-1 text").str<local>().c_str() );

*/

#ifndef __ZF_CHARCONV_HPP
#define __ZF_CHARCONV_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace charset {
    template<class Encoding = wchar_t> class conv;

  /*
      Direct wide char interface
  */

    template<> class conv<wchar_t> : public std::wstring {
    public:
        conv(const std::wstring& s)           : std::wstring(s)   { }
        conv(const wchar_t* p, std::size_t l) : std::wstring(p,l) { }
        conv(const wchar_t* p)                : std::wstring(p)   { }
        conv(std::size_t n, wchar_t c)        : std::wstring(n,c) { }
        conv(void) { }
    #if __cplusplus >= 201103L
        conv(std::wstring&& s) { swap(s); }
    #endif

        template<class E>
          std::basic_string<typename conv<E>::char_type>
            str() const { return conv<E>(*this); }

        typedef wchar_t char_type;
    };

  /*
      Any parameterization simply is a different "face" of the same class.
  */

    template<class Encoding> class conv : public conv<> {
    public:
        conv(const std::string& s)         : conv<>(decode(s.data(), s.size())) { }
        conv(const char* p, std::size_t l) : conv<>(decode(p,l)) { }
        conv(const char* p)                : conv<>(decode(p,std::strlen(p))) { }
        conv(const conv<>& other)          : conv<>(other) { }
        conv(void) { }
    #if __cplusplus >= 201103L
        conv(conv<>&& other) { swap(other); }
    #endif

        operator std::string() const
        { return encode(data(), size()); }

        template<class E>     // some compilers dont like using conv<>::str?
          std::basic_string<typename conv<E>::char_type>
            str() const { return conv<>::str<E>(); }

        typedef char char_type;
    public:           // too many compilers crap on template friend templates
        static std::wstring decode(const char*, std::size_t);
        static std::string  encode(const void*, std::size_t);
    };

  /*
      Convenient function
  */

    template<class In, class Out>
    inline std::basic_string<typename Out::char_type> recode(std::basic_string<typename In::char_type> str)
    {
        return (conv<Out>) (conv<In>) str;
    }

  /*
      Predefined charsets, explicit specialization prototypes
  */

    typedef char local;
    struct latin1;

    template<> std::wstring conv<local >::decode(const char*, std::size_t);
    template<> std::string  conv<local >::encode(const void*, std::size_t);

    template<> std::wstring conv<latin1>::decode(const char*, std::size_t);
    template<> std::string  conv<latin1>::encode(const void*, std::size_t);
}

#endif

