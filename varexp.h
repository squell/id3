/*

  Simple wildcard matching with pattern matching

  ! currently does not handle multi-byte encodings properly

  (c) 2003,2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

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

*/

#ifndef __ZF_VAREXP
#define __ZF_VAREXP

#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>
#include <iterator>

class varexp {
public:
    varexp(const char* mask, const char* test) : vars(), varl()
    { result = match(mask,test); };
    varexp() : vars(), varl()
    { result = 0; }

    operator bool() const
    { return result; }

    std::string operator[](unsigned idx) const;

    unsigned size() const
    { return vars.size(); }

    char* cpy(char* dest, unsigned idx) const;

    class iterator;
    iterator begin() const;
    iterator end() const;

protected:
    std::vector<const char*> vars;   // string portion indexes
    std::vector<int>         varl;   // string portion lengths
    bool                     result;

    bool match(const char* mask, const char* test);
    int in_set(char c, const char* set, const char* test);
};

inline std::string varexp::operator[](unsigned idx) const
{
    if( idx >= vars.size() )                  // bounds check
        throw std::out_of_range("varexp: index out of range");

    return std::string( vars[idx], varl[idx] );
}

inline char* varexp::cpy(char* dest, unsigned idx) const
{
    return std::strncpy(dest, vars[idx], varl[idx] );
}

class varexp::iterator {
    friend class varexp;
    typedef std::vector<const char*>::const_iterator vars_ptr;
    typedef std::vector<int>::const_iterator         varl_ptr;

    vars_ptr s;
    varl_ptr l;

    iterator(vars_ptr is, varl_ptr il)
    : s(is), l(il) { }
public:
    std::string operator*() const          { return std::string(*s, *l); }
    iterator& operator++()                 { return ++s, ++l, *this; }
    iterator  operator++(int)              { iterator tmp(*this);
                                             return ++s, ++l, tmp; }

    friend bool operator==(const iterator& a, const iterator& b)
    { return a.s == b.s; }
    friend bool operator!=(const iterator& a, const iterator& b)
    { return a.s != b.s; }

    typedef std::input_iterator_tag           iterator_category;
    typedef std::string                       value_type;
    typedef std::vector<int>::difference_type difference_type;
    typedef const std::string*                pointer;
    typedef const std::string&                reference;
};

inline varexp::iterator varexp::begin() const
{
    return iterator(vars.begin(), varl.begin());
}

inline varexp::iterator varexp::end() const
{
    return iterator(vars.end(), varl.end());
}

#endif

