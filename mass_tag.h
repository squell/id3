/*

  mass_tag engine class (wrapping it all up)

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

  Usage:

  Initialize with a set_tag object (containing changes to make) and a
  set_tag::read factory (selecting the data source); the result is a finder
  object which will apply changes to the files it is specified to operate on.

  Example:

    mass_tag( ID3().set(artist, "TAFKAT") ).glob("*.mp3");

*/

#ifndef __ZF_MASS_TAG_HPP
#define __ZF_MASS_TAG_HPP

#include <string>
#include "fileexp.h"
#include "set_base.h"

namespace fileexp {

    class mass_tag : public find {
    public:
        mass_tag(const set_tag::handler& write, const set_tag::provider& read)
        : counter(0), tag_update(write), tag_info(read) { }
        template<class T> mass_tag(const T& tag)
        : counter(0), tag_update(tag),   tag_info(tag)  { }

        const set_tag::handler&  tag_update;
        const set_tag::provider& tag_info;

        static set_tag::ID3field field(char c);
        static std::string       var  (int i);
        static unsigned long int total();
    protected:
        unsigned long int counter;

        virtual bool file(const char* name, const record&);
        virtual bool dir (const record&);
    };

}

#endif

