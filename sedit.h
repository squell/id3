/*

  string replacement function

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

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

class svar {
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

class sedit : svar {
public:
    sedit(std::string msg) : fmt(msg) { }

    template<class T>
      std::string operator()(const T& vars)
    { return edit(fmt, container<T>(vars)); }

private:
    const std::string fmt;
};

#endif

