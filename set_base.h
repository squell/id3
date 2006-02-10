/*

  Interface for applicative tag classes

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  Use a handler object to specify the changes to make and use the modify()
  member to make those changes to multiple files. If clear() is used and no
  changes were specified, modify() will remove any tags it encounters.

  modify() parses "%x" variables using string_parm::edit

  Restrictions:

  vmodify() should throw tag::failure on critical errors.

  virtual destructor in tag::handler and tag::provider ommitted for
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

namespace tag {

    enum ID3field {
        title, artist, album, year, cmnt, track, genre, FIELD_MAX
    };

    class handler;             // abstract base class / interface

    class combined;            // group multiple writers into one

    class reader;              // extra interface for providing readers
    class metadata;            // abc abstracting tag format

    class failure;             // exception class

}


namespace tag {

  ///////////////////////////////////////////////////////
  // interface part                                    //
  ///////////////////////////////////////////////////////

class non_copyable {
    non_copyable(const non_copyable&);
    void operator=(const non_copyable&);
protected:
    non_copyable() { }
};

class handler : non_copyable {
public:
    struct body;

    bool modify(const char* fn, const stredit::function& sub = stredit::identity()) const
    { return vmodify(fn, sub); }

    template<class T>
      bool modify(const char* fn, T& var) const
    { return p_modify<T>(fn, &var); }

  // standard state set methods

    virtual handler& set(ID3field, std::string) = 0;
    virtual handler& clear(bool = true) = 0;

    virtual handler& reserve(std::size_t req = 0)
    { return *this; }

  // free-form set methods (optional - default to no-op)

    virtual bool set(std::string, std::string)
    { return false; }
    virtual bool rm(std::string)
    { return false; }

    virtual bool from(const char* fn)
    { return false; }

protected:                     // disable outside destruction
    typedef stredit::function function;
    ~handler() { }

    virtual bool vmodify(const char*, const stredit::function&) const = 0;

private:                       // overload selectors
    template<class T>
      bool p_modify(const char* fn, const void* var) const
    { return vmodify(fn, stredit::array((T*)var)); }

    template<class T>
      bool p_modify(const char* fn, const stredit::function* var) const
    { return vmodify(fn, *var); }
};

  ///////////////////////////////////////////////////////
  // tag reading interface                             //
  ///////////////////////////////////////////////////////

class reader : non_copyable {
public:
    virtual metadata* read(const char*) const = 0;
protected:
    ~reader() { }
};

class metadata : non_copyable {
public:
    typedef stredit::function::result value_string;
    typedef std::vector< std::pair<std::string, value_string> > array;

    virtual value_string operator[](ID3field) const = 0;
    virtual array        listing()            const = 0;
    virtual operator bool()                   const = 0;
    virtual ~metadata() { }

protected:                     // a pre-defined factory
    template<class Instance> struct factory : reader {
        virtual metadata* read(const char* fn) const
        { return new Instance(fn); }
    };
};

  ///////////////////////////////////////////////////////
  // boilerplate plumbing                              //
  ///////////////////////////////////////////////////////

class handler::body {
    struct null;
public:
    body() : update(), cleared() { }

    struct nullable : private std::pair<std::string, bool> {
        void operator=(const null*)           { first.erase(), second = 0; }
        void operator=(std::string p)         { first.swap(p), second = 1; }
        operator const std::string*() const   { return second? &first : 0; }
        const std::string* operator->() const { return *this; }
    };

    nullable update[FIELD_MAX];      // modification data
    bool cleared;                    // should vmodify clear existing tag?

    void set(ID3field i, std::string m)
    { if(i < FIELD_MAX) update[i] = m; }
    void clear(bool t = true)
    { cleared = t; }
};

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // (delegates all messages to registered handlers)   //
  ///////////////////////////////////////////////////////

class combined : public handler, private std::vector<handler*> {
    mutable handler::body  data;
    mutable body::nullable basefn;
public:
  // registers a delegate tag
    combined& add(handler& h)
    { push_back(&h); return *this; }
    combined& forget(size_type pos, size_type num = 1)
    { erase(begin()+pos, begin()+pos+num); return *this; }

  // standard state set methods
    combined& clear(bool t = true)
    { data.clear(t);  return *this; }
    combined& set(ID3field i, std::string m)
    { data.set(i, m); return *this; }

    bool vmodify(const char*, const stredit::function&) const;
    bool from(const char*);

  // publish some methods
    using std::vector<handler*>::iterator;
    using std::vector<handler*>::begin;
    using std::vector<handler*>::end;
    using std::vector<handler*>::size;
    using std::vector<handler*>::operator[];
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

