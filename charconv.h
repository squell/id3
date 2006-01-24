/*

  character conversion (user locale <-> latin1)

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The cvtstring class encapsulates an encoding-neutral string. Uses templates
  heavily for avoiding compile dependencies and using the C++ type system
  itself for specifying conversions. A limited string interface is provided
  for constructing, but not editing strings.

  Example:

  std::puts( cvtstring::latin1("ISO 8859-1 text").local().c_str() );
  std::puts( conv::string<latin1>("ISO 8859-1 text").c_str<local>() );

*/

#ifndef __ZF_CHARCONV_HPP
#define __ZF_CHARCONV_HPP

#include <string>
#ifdef __DJGPP__
namespace std {
    typedef basic_string<wchar_t> wstring;
}
#endif

namespace charset {
    template<class Encoding = void> class conv;

  /*
	  Making the default template the base class for non default templates
	  solves the problem of dynamically specifying conversions, removes
	  the need for a template conversion constructor, and moves error messages
	  from the linking stage to the compiler stage.
  */

    template<> class conv<void> {
        template<class T> struct proxy {                // value wrapper
            typedef typename conv<T>::char_type char_t;
            operator const char_t*() const { return str.c_str(); }
            proxy(const std::basic_string<char_t>& s) : str(s) { }
		private:
            const std::basic_string<char_t> str;
		};
        typedef std::wstring    data;
        typedef data::size_type size_t;
	public:
        conv(const conv<>& other) : internal(other.internal) { }
        conv(void)                : internal()               { }

        bool empty() const          { return internal.empty(); }
        void clear()                { internal.erase(); }
        void swap(conv<>& other)    { internal.swap(other.internal); }

        conv<>& operator+=
          (wchar_t c)               { return internal.append((data::value_type*)&c, cellsize), *this; }
        conv<>& operator+=
          (const conv<>& rhs)       { return (internal+=rhs.internal), *this; }

        template<class E>
          std::basic_string<typename conv<E>::char_type>
                   str()   const    { return conv<E>(*this); }
        template<class E>
          proxy<E> c_str() const    { return str<E>(); }
    private:
        template<class Kin> friend class conv;
        static const int cellsize = sizeof(wchar_t) / sizeof(data::value_type);
        data internal;
        explicit conv(const data& s) : internal(s) { }
	};

    inline conv<> operator+(conv<> lhs, const conv<>& rhs)
    {
        return lhs += rhs;
    }

  /*
	  Any parameterization simply is a different "face" of the same class.
  */

    template<class Encoding> class conv : public conv<> {
	public:
        conv(const std::string& s)    : conv<>(decode(s.data(), s.size())) { }
        conv(const char* p, size_t l) : conv<>(decode(p,l)) { }
        conv(const char* p)           : conv<>((conv)std::string(p)) { }
        conv(const conv<>& other)     : conv<>(other) { }
        conv(void)                    : conv<>() { }

		operator std::string const() const
        { return encode(internal.data(), internal.size()/cellsize); }

        std::string const str() const { return *this; }
        proxy<char> c_str()     const { return str(); }

        typedef char char_type;
    private:
        static conv<>::data decode(const char*, std::size_t);
        static std::string  encode(const void*, std::size_t);
        template<class Kin> friend class conv;
	};

  /*
      Direct wide char interface
  */

    template<> class conv<wchar_t> : public conv<> {
    public:
        conv(const std::wstring& s)   : conv<>(s) { }
        conv(const wchar_t* p, size_t l) : conv<>(std::wstring(p,l)) { }
        conv(const wchar_t* p)        : conv<>(p) { }
        conv(const conv<>& other)     : conv<>(other) { }
        conv(void)                    : conv<>() { }
        conv(size_t n, wchar_t c)     : conv<>(std::wstring(n,c)) { }

        operator std::wstring const() const { return internal; }
        operator std::wstring&()            { return internal; }
        std::wstring const str()      const { return internal; }
        std::wstring& str()                 { return internal; }
  //    const wchar_t* c_str()        const { return internal.c_str(); }

        typedef wchar_t char_type;
    };

  /*
      Convenient function
  */

    template<class In, class Out>
    inline std::string recode(std::string str)
    {
        return (conv<Out>) (conv<In>) str;
    }

  /*
      Predefined charsets.
  */

    typedef char local;
    struct latin1;
}

#endif

