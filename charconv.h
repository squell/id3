/*

  character conversion (user locale -> latin1)

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  bla..

*/

#ifndef __ZF_CHARCONV_HPP
#define __ZF_CHARCONV_HPP

#include <string>

class cvtstring {
public:
    static cvtstring local (const std::string&);
    static cvtstring latin1(const std::string&);

    std::string local() const;
    std::string latin1() const;

    bool empty() const;

    static const char* system_charset();        // returns locale or NULL

    cvtstring(const std::string&);
    cvtstring(const char*);
    cvtstring();

    typedef std::string (cvtstring::*xlat)() const;

private:
    cvtstring(const std::string& s, int) : internal(s) { }
    std::string internal;

    static std::string (*conv_to_internal)(const std::string&);
    static std::string (*conv_to_locale)  (const std::string&);
    class initialize;
    static initialize instance;
    friend class initialize;                    // required by C++98
};

  // basic entry/exit shorthands

inline cvtstring::cvtstring(const std::string& s)
: internal(conv_to_internal(s))
{ }

inline cvtstring::cvtstring(const char* p)
: internal(conv_to_internal(p))
{ }

inline cvtstring::cvtstring()
: internal()
{ }

  // some functions are too simple not to inline

inline bool cvtstring::empty() const
{
    return internal.empty();
}

inline cvtstring cvtstring::local(const std::string& str)
{
    return cvtstring(conv_to_internal(str), 0);
}

inline std::string cvtstring::local() const
{
    return conv_to_locale(internal);
}

inline cvtstring cvtstring::latin1(const std::string& str)
{
    return cvtstring(str,0);
}

inline std::string cvtstring::latin1() const
{
    return internal;
}

#endif

