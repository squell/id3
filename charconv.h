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

namespace charset {
    template<class Encoding = void> class conv;

  /*
	  Making the default template the base class for non default templates
	  solves the problem of dynamically specifying conversions, removes
	  the need for a template conversion constructor, and moves error messages
	  from the linking stage to the compiler stage.
  */

    template<> class conv<void> {
        template<class Kin> friend class conv;
        typedef std::string::size_type size_t;
		struct proxy {									// value wrapper
			operator const char*() const { return str.c_str(); }
            proxy(const std::string& s) : str(s) { }
		private:
			const std::string str;
		};
	public:
        conv(const conv<>& other) : internal(other.internal) { }
        conv(void)                : internal()               { }

        bool empty() const          { return internal.empty(); }
        void clear()                { internal.erase(); }
        void swap(conv<>& other)    { internal.swap(other.internal); }

        conv<>& operator+=
          (const conv<>& rhs)       { return (internal+=rhs.internal), *this; }

		template<class E>
          std::string str() const   { return conv<E>(*this); }
		template<class E>
          proxy c_str() const       { return str<E>(); }
	private:
		std::string internal;
        explicit conv(const std::string& s) : internal(s) { }
	};

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
		{ return encode(internal.data(), internal.size()); }

    /*  using conv<>::str;
        using conv<>::c_str; */
		std::string str() const { return *this; }
        proxy c_str()     const { return str(); }
	private:
		static std::string decode(const char*, std::size_t);
		static std::string encode(const char*, std::size_t);
        template<class Kin> friend class conv;
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

    struct local;
    struct latin1;
}

#endif

