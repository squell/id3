/*

  string replacement function

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  string_parm::edit parses any "%x"'s found in its first argument and
  replaces them with corresponding entries from containers passed in its
  second argument.

  Optional modifiers:
   "+"     Pass Throught A Capitalization Function
   "-"     convert to lower case
   "_"     Don't_eliminate_underscores_and   don't   compress    spaces
   "#"     Pad numbers with (number of #'s-1) zero's.
   "|alt|" Replace with alternate text if substitution would fail.

  Special forms:
   "%%" -> %
   "%," -> \n

  Restrictions:

  If using the string_parm::container<> template, the only requirements of the
  container types passed to it are that they have a [] operator defined, and
  that it return data convertible into a cvtstring. E.g. a standard C
  "array-of-char*" will do, as will std::vector<string>.

  If a container does not perform bounds checking on the [] operator, the
  results of using an out-of-bounds %x substitution directed to it container
  is undefined.

  Example:

      char* tab[] = { "foo", "bar" };
      cout << capitalize( sedit("%2%1", tab) );   // "Barfoo"

*/

#ifndef __ZF_SEDIT_HPP
#define __ZF_SEDIT_HPP

#include <string>
#include "charconv.h"

extern std::string capitalize(std::string s);

template<class T>
  inline std::string sedit(const std::string&, const T&);
template<class T, class U>
  inline std::string sedit(const std::string&, const T&, const U&);

  //

class string_parm {
    static const bool ZERO_BASED = false;     // count starts at %0 ?
    static const char VAR = '%';              // replacement char

protected:
    struct subst {
        virtual cvtstring numeric(unsigned) const = 0;
        virtual cvtstring alpha  (char)     const = 0;
    };

    template<class T, class U>                // templatized wrapper
    struct container : subst {
        const T& num;
        const U& chr;

        container(const T& t, const U& u) : num(t), chr(u) { }

        virtual cvtstring numeric(unsigned x) const { return num[x]; }
        virtual cvtstring alpha  (char x)     const { return chr[x]; }
    };

    struct dummy {
        const char* operator[](unsigned) const { return ""; }
    };

    static cvtstring edit
      (const cvtstring&, const subst&, const char* = "", bool = 0);

    template<class T>
      friend std::string sedit(const std::string&, const T&);
    template<class T, class U>
      friend std::string sedit(const std::string&, const T&, const U&);
};

  // little excuse for making this a useful header :)

template<class T>
  inline std::string sedit(const std::string& fmt, const T& vars)
{
    return sedit(fmt, vars, dummy());
}

template<class T, class U>
  inline std::string sedit(const std::string& fmt, const T& vars, const U& table)
{
    return string_parm::edit(fmt,
      string_parm::container<T,U>(vars, table)).local();
}

#endif

