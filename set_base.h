/*

  Interface for applicative tag classes

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  Use a handler object to specify the changes to make and use the modify()
  member to make those changes to multiple files. If clear() is used and no
  changes were specified, modify() will remove any tags it encounters.

  modify() parses "%x" variables using string_parm::edit

  set_tag::group calls multiple tags with the same values, in the reverse
  order of them being registered. (*)

  Restrictions:

  vmodify() should throw set_tag::failure on critical errors.

  virtual destructor in set_tag::handler and set_tag::provider ommitted for
  filesize issues with gcc3.

*/

#ifndef __ZF_SET_BASE_HPP
#define __ZF_SET_BASE_HPP

#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <exception>
#include <new>
#include "sedit.h"

namespace set_tag {

    enum ID3field {
        title, artist, album, year, cmnt, track, genre, FIELDS
    };

    class handler;             // abstract base class / interface

    class group;               // group multiple writers into one

    class provider;            // extra interface for providing readers
    class reader;              // abc abstracting tag format

    class failure;             // exception class

}


namespace set_tag {            // borland likes this better

  ///////////////////////////////////////////////////////
  // interface part                                    //
  ///////////////////////////////////////////////////////

class handler : protected string_parm {
public:
    virtual bool vmodify(const char*, const subst&) const = 0;
    struct body;

    template<class T, class U>
      bool modify(const char* fn, T& vars, U& table) const
    { return vmodify(fn, container<T,U>(vars, table)); }

    template<class T, class U>
      bool modify(const char* fn, const T& vars, const U& table) const
    { return vmodify(fn, container<const T, const U>(vars, table)); }

    template<class T>
      bool modify(const char* fn, const T& vars) const
    { return modify(fn, vars, dummy()); }

  // standard state set methods

    virtual handler& set(ID3field, std::string) = 0;
    virtual handler& clear() = 0;

    virtual handler& reserve(std::size_t req = 0)
    { return *this; }

  // free-form set methods (optional - default to no-op)

    virtual bool set(std::string, std::string)
    { return false; }
    virtual bool rm(std::string)
    { return false; }

protected:                     // disable outside destruction and copying
    handler() { }
    ~handler() { }

private:
    void operator=(const handler&);
    handler(const handler&);
};

  ///////////////////////////////////////////////////////
  // tag reading interface                             //
  ///////////////////////////////////////////////////////

class provider {
public:
    virtual const reader* read(const char*) const = 0;
protected:
    ~provider() { }
};

class reader {
public:
    reader() { }
    typedef std::vector< std::pair<std::string, cvtstring> > array;

    virtual cvtstring operator[](ID3field) const = 0;
    virtual array     listing() const = 0;
    virtual ~reader() { }

private:                       // prevent copying of classes
    void operator=(const reader&);
    reader(const reader&);
};

  ///////////////////////////////////////////////////////
  // boilerplate plumbing                              //
  ///////////////////////////////////////////////////////

struct handler::body {
    body() : update(), cleared() { }
private:
    struct null;
    struct nullable : private std::pair<std::string, bool> {
        void operator=(const null*)         { first.erase(), second = 0; }
        void operator=(std::string p)       { first.swap(p), second = 1; }
        operator const std::string*() const { return second? &first : 0; }
    };
public:
    nullable update[FIELDS];         // modification data
    bool cleared;                    // should vmodify clear existing tag?
};

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // (delegates all messages to registered handlers)   //
  ///////////////////////////////////////////////////////

class group : public handler {
    std::vector<handler*> reg;
    mutable handler::body data;
public:
  // registers a delegate tag
    group& delegate(handler& h);

  // standard state set methods (non-inline)
    group& clear()
    { data.cleared = true; return *this; }
    group& set(ID3field i, std::string m)
    { if(i < FIELDS) data.update[i] = m; return *this; }

    bool vmodify(const char*, const subst&) const;

  // inspect
    const std::vector<handler*>& handlers() const
    { return reg; }
};

  ///////////////////////////////////////////////////////
  // severe error reporting                            //
  ///////////////////////////////////////////////////////

class failure : public std::exception {
    mutable char* txt;
public:
    explicit failure(const char* err);
    explicit failure(const char* err, const char* context);
    explicit failure(const std::string& err);
    failure(const failure& other)
    { *this = other; }
    void operator=(const failure& other)
    { txt = other.txt; other.txt = 0; }
    virtual ~failure() throw()
    { delete[] txt; }
    virtual const char* what() const throw()
    { return txt ? txt : "<null>"; }
};

 /*
    I'm NOT throwing a std::string here because:
    - Can't simply pass the reference we're given (won't be valid soon)
    - Don't like the theoretical possibility of a copy constructor throwing
      an exception while I'm throwing an exception.
    - Dynamically allocating a string and passing that pointer around is
      more work :)
  */

inline failure::failure(const std::string& err)
{
    using namespace std;
    txt = new (nothrow) char[err.length()+1];
    if(txt) {
        strcpy(txt, err.c_str());
    }
}

inline failure::failure(const char* err)
{
    using namespace std;
    txt = new (nothrow) char[strlen(err)+1];
    if(txt) {
        strcpy(txt, err);
    }
}

inline failure::failure(const char* err, const char* context)
{
    using namespace std;
    size_t elen = strlen(err);
    size_t clen = strlen(context);
    txt = new (nothrow) char[elen+clen+1];
    if(txt) {
        strcpy(txt,      err);
        strcpy(txt+elen, context);
    }
}

}

#endif

