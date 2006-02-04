#include <string>
#include <stdexcept>
#include "set_base.h"
#include "mass_tag.h"
#include "sedit.h"
#include "pattern.h"

/*

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

*/

using namespace tag;
using fileexp::mass_tag;
using std::string;

 // "inside out" way of specifying what you want.
 // - kind of lazy. but hey long live code reuse :)
 // - assumes sedit() processes variables left-to-right

namespace {
    static char error[] = "illegal variable: %_";

    struct counter {
        handler* tag;               // Borland doesn't like ref's in aggr's?
        char var[3];
        bool w;

        const char* operator()(char);
    };

    const char* counter::operator()(char c)
    {
        if(var[1]++ == '9')                        // limit reached
            throw std::out_of_range("too many variables in pattern");
        ID3field field = mass_tag::field(c);
        if(field < tag::FIELD_MAX) {
            w = true, tag->set(field, var);
        } else if(c == 'x') {
            ;                                      // pass over in silence
        } else {
            error[sizeof error-2] = c;
            throw std::out_of_range(error);
        }
        return "*";
    }
}

pattern::pattern(handler& tag, std::string mask)
{
    string::size_type pos(0);                  // replace '*' with stubs
    while((pos = mask.find('*',pos)) != string::npos) {
        mask.replace(pos, 1, "%x");
    }
    counter var = { &tag, "%0" };
    str = stredit::wrap(var)(mask);
    ok  = var.w;
}

