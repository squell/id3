/*

  smartID3 applicative class

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The smartID3 class defines an object than can patch ID3 tags in files.

  You create a smartID3 object, tell it the changes you want to make to an
  ID3 tag, and can then use the "modify()" member to make such changes to as
  many files as you'd like. modify() parses any "%x"'s found in the field
  strings and replaces them with corresponding entries from the container you
  pass as its second argument.

  A "%x" may have an optional "c" modifier which passes the results of such
  a substitution Through A Capitilization Function. Normally, any underscores
  will be replaced with spaces, but_this_can_be_suppressed_by_adding the "_"
  modifier. "%%" will be replaced by a single % character, and "%:" will be
  replaced with a \0 character, "%n" becomes \n.

  If you use clear() and don't specify any changes, modify() will remove any
  ID3 tags it encounters.

  In addition, this header defines a enumeration ID3set, which basically
  serves no real purpose other than to make sourcecode more enjoyable. :)

  Restrictions:

  The only requirements of the container type is that it has a [] operator
  defined, and that it contains data that can be converted into a std:string.
  A standard C "array-of-char*" will do, as will std::vector<string>.

  If your container type does not perform bounds checking on the [] operator,
  the results of using an out-of-bounds %x substitution is undefined.

  smartID3 only supports ID3v1 tags. It should not be hard to write a
  derivative class that handles ID3v2, though - redefine vmodify().

  The failure class should be used only to report errors.

  Example:

  int main(int argc, char* argv[])
  {
      smartID3()
      .set(artist, "%2")
      .set(title,  "%3")
      .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETID3
#define __ZF_SETID3

#include <string>
#include <vector>
#include <exception>
#include <memory>
#include "sedit.h"

enum ID3set {
    title, artist, album, year, cmnt, track, genre, ID3_MAX
};

class smartID3 : protected svar {
    std::vector<const char*> mod;

protected:
    bool fresh;                               // clear existing?

    virtual bool vmodify(const char*, const base_container&) const;

public:
    smartID3() : mod(ID3_MAX,(char*)0), fresh(false)
    { }

    smartID3& set(ID3set i, const char* m)    // set ID3 field i to value m
    { if(i < ID3_MAX) mod[i] = m; return *this; }

    smartID3& clear()                         // clear original ID3 tag
    { fresh = true; return *this; }

    template<class T>                         // returns success/failure
    bool modify(const char* fn, const T& vars) const
    { return vmodify(fn, container<T>(vars)); }

    class failure;
};

class smartID3::failure : public std::exception {
    mutable std::auto_ptr<char> txt;
public:
    explicit failure(const std::string&);
    failure(const failure& other) : txt( other.txt )
    { }
    virtual ~failure() throw()
    { }
    virtual const char* what() const throw()
    { return txt.get() ? txt.get() : "<null>"; }
};

#endif

