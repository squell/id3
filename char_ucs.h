/*

  character conversion (<-> unicode encodings)

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

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

    template<byte_order> struct unicode { };

    typedef unicode<marked>        ucs2;
    typedef unicode<little_endian> ucs2le;
    typedef unicode<big_endian>    ucs2be;

    class conv_ucs {
    protected:
        static conv<>::data decode(const char*, std::size_t, byte_order);
        static std::string  encode(const void*, std::size_t, byte_order);
    };

    template<byte_order Order>
      class conv< unicode<Order> > : public conv<>, private conv_ucs {
	public:
        friend class conv_ucs;
        conv(const std::string& s)         : conv<>(decode(s.data(), s.size())) { }
        conv(const char* p, std::size_t l) : conv<>(decode(p,l)) { }
        conv(const conv<>& other)          : conv<>(other) { }
        conv(void)                         : conv<>() { }

		operator std::string const() const
        { return encode(internal.data(), internal.size()/cellsize); }

        std::string const str() const { return *this; }

        template<class E>
          std::basic_string<typename conv<E>::char_type>
                   str()   const    { return conv<>::str<E>(); }
        template<class E>
          proxy<E> c_str() const    { return conv<>::c_str<E>(); }

        typedef char char_type;
    private:
        inline static conv<>::data decode(const char* p, std::size_t len)
        { return conv_ucs::decode(p, len, Order); }
        inline static std::string  encode(const void* p, std::size_t len)
        { return conv_ucs::encode(p, len, Order); }
        template<class Kin> friend class charset::conv;
    };

}

#endif

