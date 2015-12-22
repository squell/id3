/*

  character conversion (<-> unicode encodings)

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage: see charconv.h

  Uses partial specialization to encode various characteristics.

  Issues: For some reason, Borland C++ doesn't like this

    charset::conv<char>("Hello").str<charset::ucs2>();

*/

#ifndef __ZF_CHARCONV_UNICODE_HPP
#define __ZF_CHARCONV_UNICODE_HPP

#include "charconv.h"

namespace charset {

    enum byte_order { marked, little_endian, big_endian };

    template<byte_order = marked> class unicode;

    typedef unicode<marked>        utf16;
    typedef unicode<little_endian> utf16le;
    typedef unicode<big_endian>    utf16be;

    class conv_wide : public conv<> {
    protected:
        template<class T>
          conv_wide(const T& s) : conv<>(s) { }
        conv_wide() { }
       ~conv_wide() { }
        static std::wstring decode(const char*, std::size_t, byte_order);
        static std::string  encode(const void*, std::size_t, byte_order);
        static std::size_t  ucslen(const char* p)
        {
            for(std::size_t len = 0; ; len += 2)
                if( (p[len]|p[len+1]) == 0 )
                    return len;
        }
    };

    template<byte_order Order>
      class conv< unicode<Order> > : public conv_wide {
    public:
        conv(const std::string& s)         : conv_wide(decode(s.data(), s.size(),Order)) { }
        conv(const char* p, std::size_t l) : conv_wide(decode(p,l,Order)) { }
        conv(const char* p)                : conv_wide(decode(p,ucslen(p),Order)) { }
        conv(const conv<>& other)          : conv_wide(other) { }
        conv(void)                         : conv_wide() { }

        operator std::string() const
        { return encode(data(), size(), Order); }

        template<class E>
          std::basic_string<typename conv<E>::char_type>
            str() const { return conv<>::str<E>(); }

        typedef char char_type;
    };

}

#endif

