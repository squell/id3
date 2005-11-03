#include <string>
#include <stdexcept>
#include "pattern.h"
#include "set_base.h"
#include "mass_tag.h"

/*

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace set_tag;
using fileexp::mass_tag;
using std::string;

 // "inside out" way of specifying what you want.
 // - kind of lazy. but hey long live code reuse :)
 // - assumes sedit() processes variables left-to-right

namespace {
    static char error[] = "illegal variable: %_";

    struct count {
        handler* tag;               // Borland doesn't like ref's in aggr's?
        char var[3];
        bool w;

        const char* operator[](char);
        const char* operator[](unsigned);
    };

    const char* count::operator[](char c)
    {
        if(var[1]++ == '9')                        // limit reached
            throw std::out_of_range("too many variables in pattern");
        ID3field field = mass_tag::field(c);
        if(field < set_tag::FIELDS) {
            w = true, tag->set(field, var);
        } else if(c == 'x') {
            ;                                      // pass over in silence
        } else {
            error[sizeof error-2] = c;
            throw std::out_of_range(error);
        }
        return "*";
    }

    const char* count::operator[](unsigned i)
    {
        error[sizeof error-2] = (i+1) % 10 + '0';
        throw std::out_of_range(error);
    }
}

pattern::pattern(handler& tag, std::string mask)
{
    string::size_type pos(0);                  // replace '*' with stubs
    while((pos = mask.find('*',pos)) != string::npos) {
        mask.replace(pos, 1, "%x");
    }
    count var = { &tag, "%0" };
    str = sedit(mask, var, var);
    ok  = var.w;
}

