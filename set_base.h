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

    class writer;              // interface for writing to files
    class reader;              // interface for providing readers
    class handler;             // abstract base class

    class metadata;            // class abstracting tag info

    class failure;             // exception class

}


namespace tag {

  ///////////////////////////////////////////////////////
  // tag writing interface                             //
  ///////////////////////////////////////////////////////

class writer {
public:
    typedef stredit::function function;

    bool modify(const char* fn, const function& sub = stredit::identity()) const
    { return vmodify(fn, sub); }

    template<class T> bool modify(const char* fn, const T* var) const
    { return vmodify(fn, stredit::array(var)); }

protected:
    virtual bool vmodify(const char*, const function&) const = 0;
    ~writer() { }              // disable outside destruction
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
  // tag setting/getting interface                     //
  ///////////////////////////////////////////////////////

class handler : public writer {
public:

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

