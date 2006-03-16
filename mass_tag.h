/*

  mass_tag engine class (wrapping it all up)

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  Initialize with a tag object (containing changes to make) and a tag::read
  factory (selecting the data source); the result is a finder object which
  will apply changes to the files it is specified to operate on.

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
        mass_tag(const tag::writer& write, const tag::reader& read)
        : tag_writer(write), tag_reader(read), counter(0) { }
        template<class T> mass_tag(const T& tag)
        : tag_writer(tag),   tag_reader(tag), counter(0) { }

        const tag::writer& tag_writer;
        const tag::reader& tag_reader;

        static tag::ID3field field(wchar_t c);
        static std::string   var  (int i);
        static unsigned long total();
    protected:
        unsigned long counter;

        virtual bool file(const char* name, const record&);
        virtual bool dir (const record&);
    };

}

#endif

