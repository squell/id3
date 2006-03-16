/*

  string replacement function

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  stredit::function is an abtract class for string-to-string mapping

  Using stredit::format, any "%x"'s found will be processed; actual variables
  will be passed on to a descendant class.

  Optional modifiers to %x:
   "+"     Pass Throught A Capitalization Function
   "-"     convert to lower case
   "_"     Don't_eliminate_underscores_and   don't   compress    spaces
   "#"     Pad numbers with (number of #'s-1) zero's.
   "|alt|" Replace with alternate text if substitution would fail.

  Special forms:
   "%%" -> %
   "%?" -> used for %| alt || alt? || alt? |?

  stredit::array() will instantiate a formatter which looks up %0 to %9 in
  the array passed as its argument.

  Example:

      char* tab[] = { "foo", "bar" };
      string s = array(tab)("%1, %+0");   // "bar, Foo"

*/

#ifndef __ZF_NSEDIT_HPP
#define __ZF_NSEDIT_HPP

#include <string>
#include "charconv.h"

namespace stredit {

    class function {
    public:
        struct result : charset::conv<char> {
            result(const charset::conv<>& s, bool ok)
            : charset::conv<char>(s), m_good(ok) { }
            result(const charset::conv<>& s)
            : charset::conv<char>(s), m_good(!s.empty()) { }
            result(const std::string& s)
            : charset::conv<char>(s), m_good(!s.empty()) { }
            result(const char* p)
            : charset::conv<char>(p), m_good(*p) { }
            result(int ok = 0) : m_good(ok) { }
            bool good() const { return m_good; }
            operator charset::conv<char> const*() const
            { return good()? this : 0; }
        private:
            bool m_good;
        };

        virtual result operator()(const charset::conv<char>&) const = 0;
    };

    class identity : public function {
        virtual result operator()(const charset::conv<char>& s) const
        { return result(s,1); }
    };

    class format : public function {
    public:
        static char const prefix = '%';

        virtual result operator()(const charset::conv<char>& s) const
        { return edit(charset::conv<wchar_t>(s), false); }
    protected:
        result edit(const std::wstring&, bool atomic) const;

        typedef std::wstring::const_iterator ptr;
      // on entry, p != end
        virtual result code (ptr& p, ptr end) const;
        virtual result var  (ptr& p, ptr end) const = 0;
        virtual ptr matching(ptr p,  ptr end) const;
    };

    template<class T> struct format_wrapper : format {
        format_wrapper(T& i) : f(i) { }
        virtual result var(ptr& p, ptr) const
        { return (charset::conv<char>) f(*p++); }
    private:
        T& f;
    };

    template<class F>
      format_wrapper<F> wrap(F& fun) { return fun; }

    template<class T> struct format_array : format {
        format_array(T i) : cont(i) { }
        virtual result var(ptr& p, ptr) const
        { return (charset::conv<char>) cont[*p++ - '0']; }
    private:
        T cont;
    };

    template<class A>
      format_array<A&> array(A& arr) { return format_array<A&>(arr); }
    template<class A>
      format_array<A*> array(A* arr) { return format_array<A*>(arr); }
}

#endif

