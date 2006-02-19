/*

  Interface for applicative tag classes

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  Use a handler object to specify the changes to make and use the modify()
  member to make those changes to multiple files. If rewrite() is used and no
  changes were specified, modify() will remove any tags it encounters. To add
  tags to files that don't already have them, also use create()

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

class handler {
public:
    typedef stredit::function function;
    struct body;

    bool modify(const char* fn, const function& sub = stredit::identity()) const
    { return vmodify(fn, sub); }

    template<class T> bool modify(const char* fn, const T* var) const
    { return vmodify(fn, stredit::array(var)); }

  // standard state set methods

    virtual handler& set(ID3field, std::string) = 0;
    virtual handler& rewrite(bool = true) = 0;
    virtual handler& create(bool = true) = 0;

  // free-form set methods (optional - default to no-op)

    virtual handler& reserve(std::size_t req = 0)
    { return *this; }
    virtual bool set(std::string, std::string)
    { return false; }
    virtual bool rm(std::string)
    { return false; }
    virtual bool from(const char* fn)
    { return false; }

protected:
    ~handler() { }             // disable outside destruction

    virtual bool vmodify(const char*, const function&) const = 0;
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
  // - mimics but does not override handler            //
  ///////////////////////////////////////////////////////

class handler::body {
    struct null;
public:
    body() : update(), cleared(), generate(1) { }

    struct nullable : private std::pair<std::string, bool> {
        void operator=(const null*)           { first.erase(), second = 0; }
        void operator=(std::string p)         { first.swap(p), second = 1; }
        operator const std::string*() const   { return second? &first : 0; }
        const std::string* operator->() const { return *this; }
    };

    nullable update[FIELD_MAX];      // modification data
    bool cleared;                    // should vmodify clear existing tag?
    bool generate;                   // don't add tags to files without them?

    body& set(ID3field i, std::string m)
    { if(i < FIELD_MAX) update[i] = m; return *this; }
    body& rewrite(bool t = true)
    { cleared = t;  return *this; }
    body& create(bool t = true)
    { generate = t; return *this; }
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

