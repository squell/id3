/*

  Simple wildcard matching with pattern matching

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  Construct a varexp object using a wildcard specification and a test
  string, and then access the matched variables (if any) using the
  index operator. Example:

     varexp v("*, *", "foo, bar")
     v[0]; // == "foo"
     v[1]; // == "bar"

  The index operator returns a std::string. Use the cpy() member if you
  want a copy in plain-old-C style.

  In a boolean context, a varexp object converts to a boolean value which
  holds the results of the pattern match (failed/succeed). Thus, varexp can
  also be used like an ordinary wildcard test:

     if( varexp("r*.txt", str) )
         ...       // executes if str matched "r*.txt"

  Restrictions:

  The result of using operator[] or cpy() is undefined if the string used
  to  generate the varexp object is changed. However, values returned before
  the point of alteration will remain safe.

  operator[] performs range checking, and throws an out_of_range exception
  if an index is not >= 0 && < size()

  cpy(), true to form, will not perform any range checking and is a
  potential can of worms, generally. :)

  The wildcard match will fail in certain cases where filenames are in a
  multi-byte encodings that is not filesystem-safe, like EUC-JP.

*/

#ifndef __ZF_VAREXP
#define __ZF_VAREXP

#include <cstring>
#include <vector>
#include <utility>
#include <string>
#include <stdexcept>
#include <iterator>

class varexp {
public:
    varexp(const char* mask, const char* test) : var()
    { result = match(mask,test); };
    varexp() : var()
    { result = 0; }

    operator bool() const
    { return result; }

    std::string operator[](std::size_t i) const;

    std::size_t size() const
    { return var.size(); }

    char* cpy(char* dest, std::size_t i) const;

    class iterator;
    iterator begin() const;
    iterator end() const;

protected:
    typedef std::vector< std::pair<const char*, int> > pairvec;
    pairvec var;
    bool result;

    bool match(const char* mask, const char* test);
    const char* in_set(char c, const char* set);
    friend class iterator;
};

inline std::string varexp::operator[](std::size_t i) const
{
    if( i >= var.size() )                     // bounds check
        throw std::out_of_range("varexp: index out of range");

    return std::string( var[i].first, var[i].second );
}

inline char* varexp::cpy(char* dest, std::size_t i) const
{
    return std::strncpy(dest, var[i].first, var[i].second );
}

class varexp::iterator : public std::iterator_traits<pairvec::const_iterator> {
    friend class varexp;
    pairvec::const_iterator p;

    iterator(const pairvec::const_iterator& _p) : p(_p) { }
public:
    std::string operator*() const { return std::string(p->first, p->second); }
    iterator& operator++()        { return ++p, *this; }
    iterator  operator++(int)     { iterator tmp(*this);
                                    return ++p, tmp; }

    friend bool operator==(const iterator& a, const iterator& b)
    { return a.p == b.p; }
    friend bool operator!=(const iterator& a, const iterator& b)
    { return a.p != b.p; }

    typedef std::input_iterator_tag iterator_category;
    typedef std::string             value_type;
    typedef const std::string*      pointer;
    typedef const std::string&      reference;
};

inline varexp::iterator varexp::begin() const
{
    return iterator(var.begin());
}

inline varexp::iterator varexp::end() const
{
    return iterator(var.end());
}

#endif

