/*

  string replacement function

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  string_parm::edit parses any "%x"'s found in its first argument and
  replaces them with corresponding entries from the container passed as its
  second argument.

  Optional modifiers:
   "+" Pass Throught A Capitalization Function
   "-" convert to lower case
   "_" Don't_eliminate_underscores_and   don't   compress    spaces

  Special forms:
   "%%" -> %
   "%@" -> \0
   "%," -> \n

  Restrictions:

  The only requirements of the container type is that it has a [] operator
  defined, and that it contains data that can be converted into a std:string.
  A standard C "array-of-char*" will do, as will std::vector<string>.

  If the container type does not perform bounds checking on the [] operator,
  the results of using an out-of-bounds %x substitution is undefined.

  Example:

      sedit ed("%2%1");
      char* tab[] = { "foo", "bar" };
      cout << capitalize( ed(tab) );   // "Barfoo"

*/

#ifndef __ZF_SEDIT_HPP
#define __ZF_SEDIT_HPP

#include <string>

extern std::string capitalize(std::string s);

class sedit;

  //

class string_parm {
    static const bool ZERO_BASED = false;     // count starts at %0 ?
    static const char VAR = '%';              // replacement char

protected:
    struct base_container {
        virtual std::string operator[](unsigned) const = 0;
    };

    template<class T>                         // templatized wrapper
    struct container : base_container {
        const T& data;

        container(const T& t) : data(t) { }

        virtual std::string operator[](unsigned x) const
        { return data[x]; }
    };

    static std::string edit(std::string, const base_container&);
};

  // little excuse for making this a useful header :)

class sedit : string_parm {
public:
    sedit(std::string msg) : fmt(msg) { }

    template<class T>
      std::string operator()(const T& vars)
    { return edit(fmt, container<T>(vars)); }

private:
    const std::string fmt;
};

#endif

