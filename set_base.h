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

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
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

class reader {
public:
    virtual metadata* read(const char*) const = 0;
protected:
    ~reader() { }
};

class metadata {
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
  // severe error reporting                            //
  ///////////////////////////////////////////////////////

class failure : public std::runtime_error {
public:
    explicit failure(const std::string& err, const std::string& extra = std::string())
    : std::runtime_error(err + extra) { }
};

}

#endif

