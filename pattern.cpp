#include <string>
#include <stdexcept>
#include "set_base.h"
#include "mass_tag.h"
#include "sedit.h"
#include "pattern.h"

/*

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace tag;
using fileexp::mass_tag;
using std::string;
using std::wstring;
using charset::conv;

 // "inside out" way of specifying what you want.
 // - kind of lazy. but hey long live code reuse :)
 // - assumes sedit() processes variables left-to-right

namespace {
    struct counter : stredit::format {
        handler* tag;               // Borland doesn't like ref's in aggr's?
        mutable unsigned w;
        mutable ptr mod;

        counter(handler* h) : tag(h), w(0) { }
        virtual result var  (ptr& p, ptr end) const;
        virtual result code (ptr& p, ptr end) const
        { mod = p; return stredit::format::code(p, end); }
    };

    counter::result counter::var(ptr& p, ptr) const
    {
        if(++w == 10)                              // limit reached
            throw std::out_of_range("too many variables in pattern");
        ID3field field = mass_tag::field(*p);
        if(field < tag::FIELD_MAX) {
            const string& pre = conv<wchar_t>(wstring(mod,p)).str<char>();
            tag->set(field, pre + char('0'+w));
        } else if(*p == 'x') {
            ;                                      // pass over in silence
        } else {
            static const char error[] = "illegal variable: %";
            throw std::out_of_range(error+conv<wchar_t>(&*p, 1).str<char>());
        }
        return ++p, "*";
    }
}

pattern::pattern(handler& tag, std::string mask)
{
    string::size_type pos(0);                  // replace '*' with stubs
    while((pos = mask.find('*',pos)) != string::npos) {
        mask.replace(pos, 1, "%x");
    }
    counter var(&tag);
    this->assign( var(mask) );
    num = var.w;
}

