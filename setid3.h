/*

  smartID3 applicative class

  (c) 2000 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The smartID3 class defines an object than can patch ID3 tags in files.

  You create a smartID3 object, tell it the changes you want to make to an
  ID3 tag, and can then use the "modify()" member to make such changes to as
  many files as you'd like. modify() parses any "%x"'s found in the field
  strings and replaces them with corresponding entries from the container you
  pass as its second argument. A "%x" may have an optional "c" modifier which
  passes the results of such a substitution through a capitilization function.

  If you use clear() and don't specify any changes, modify() will remove any
  ID3 tags it encounters.

  In addition, this header defines a enumeration ID3set, which basically
  serves no real purpose other than to make sourcecode more enjoyable. :)

  Restrictions:

  Any underscores in the ID3 fields are always replaced by spaces. Sorry. :)

  The only requirements of the container type is that it has a [] operator
  defined, and that it contains data that can be converted into a std:string.
  A standard C "array-of-char*" will do, as will std::vector<string>.

  If your container type does not perform bounds checking on the [] operator,
  the results of using an out-of-bounds %x substitution is undefined.

  smartID3 only supports ID3v1 tags. It should not be hard to write a
  derivative class that handles ID3v2, though - redefine vmodify().

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

enum ID3set {
    title, artist, album, year, cmnt, track, genre, ID3
};

class smartID3 {
    std::vector<const char*> mod;

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

    bool fresh;                               // clear existing?

    virtual bool vmodify(const char*, const base_container&);

public:
    smartID3() : mod(ID3,(char*)0), fresh(false)
    { }

    smartID3& set(int i, const char* m)       // set ID3 field i to value m
    { mod[i] = m;   return *this; }

    smartID3& clear()                         // clear original ID3 tag
    { fresh = true; return *this; }

    template<class T>                         // returns success/failure
    bool modify(const char* fn, const T& vars)
    { return vmodify(fn, container<T>(vars)); }

    static std::string edit(std::string, const base_container&);
};

#endif

