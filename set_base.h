/*

  Interface for applicative tag classes

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  Use a handler object to specify the changes to make and use the modify()
  member to make those changes to multiple files. If clear() is used and no
  changes were specified, modify() will remove any tags it encounters.

  modify() parses "%x" variables using string_parm::edit

  Restrictions:

  vmodify() should only return when there is a consistent state (e.g. the
  operation has succeeded completely,or failed completely). In all other
  cases it should throw set_tag::failure.

*/

#ifndef __ZF_SET_BASE_HPP
#define __ZF_SET_BASE_HPP

#include <string>
#include <exception>
#include <vector>
#include "sedit.h"

namespace set_tag {

    enum ID3field {
        title, artist, album, year, cmnt, track, genre, FIELDS
    };

    class handler;             // abstract base class / interface

    class single_tag;
    class combined_tag;

    class failure;             // exception class

}

#ifdef __BORLANDC__
namespace set_tag {            // (borland craps w/o this?)
#endif

  ///////////////////////////////////////////////////////
  // interface part                                    //
  ///////////////////////////////////////////////////////

class set_tag::handler : protected string_parm {
public:
    virtual bool vmodify(const char*, const base_container&) const = 0;

    template<class T>
      bool modify(const char* fn, const T& vars) const
    { return vmodify(fn, container<T>(vars)); }

    handler& enable()  { return active(1); }
    handler& disable() { return active(0); }

  // standard state set methods

    virtual handler& active(bool) = 0;
    virtual bool     active() const = 0;

    virtual handler& set(ID3field, const char*) = 0;
    virtual handler& clear() = 0;

  // free-form set methods (optional - default to no-op)

    virtual handler& set(std::string, std::string)
    { return *this; }
    virtual handler& rm(std::string)
    { return *this; }
};

  ///////////////////////////////////////////////////////
  // less abstract base class (seperate common code)   //
  ///////////////////////////////////////////////////////

class set_tag::single_tag : public handler {
protected:
    single_tag(bool t = true)
    : enabled(t), fresh(false) { }

    bool enabled;                  // should vmodify do anything?
    bool fresh;                    // should vmodify clear existing tag?
public:
    handler& active(bool on) { enabled = on; return *this; }
    bool     active() const  { return enabled; }

    handler& clear()         { fresh = true; return *this; }
};

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // (delegates all messages to registered handlers)   //
  ///////////////////////////////////////////////////////

class set_tag::combined_tag : public handler {
    std::vector<set_tag::handler*> tags;
public:
  // registers a delegate tag
    combined_tag& delegate(set_tag::handler& t)
    { tags.push_back(&t); return *this; }

  // standard state set methods (non-inline)
    handler& active(bool);
    bool     active() const;
    handler& set(ID3field, const char*);
    handler& clear();
};

  ///////////////////////////////////////////////////////
  // severe error reporting                            //
  ///////////////////////////////////////////////////////

class set_tag::failure : public std::exception {
    mutable char* txt;
public:
    explicit failure(const std::string&);
    failure(const failure& other) : txt( other.txt )
    { other.txt = 0; }
    virtual ~failure() throw()
    { delete[] txt; }
    virtual const char* what() const throw()
    { return txt ? txt : "<null>"; }
};

#ifdef __BORLANDC__
}
#endif

#endif

