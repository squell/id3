/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#include "set_base.h"

using namespace std;

namespace set_tag {

group& group::clear(const char* fn)
{
    fn? basefn = fn : basefn = 0;
    data.cleared = true;
    return *this;
}

group& group::set(ID3field i, std::string m)
{
    data.set(i, m);
    return *this;
}


 /* This class does NOT delegate the free form methods. This is simply
    because all tag formats use different naming conventions, so it would
    be really pointless anyhow. */

 /* Implementation of group vmodify()
    - obeys the vmodify restrictions of set_base.h */

bool group::vmodify(const char* fn, const function& val) const
{
    for(int n = 0; n < FIELDS; ++n)
        if(const string* txt = data.update[n]) {
            for(const_iterator p = begin(); p != end(); ) {
                (*p++)->set(ID3field(n), *txt);
            }
            data.update[n] = 0;
        }

    if(data.cleared) {
        const char* s = basefn? basefn->c_str() : 0;
        for(const_iterator p = begin(); p != end(); ) (*p++)->clear(s);
        data.cleared = false;
    }

    bool ok = true;                      // process delegates
    for(const_iterator p = begin(); p != end(); ) {
        ok &= (*p++)->vmodify(fn, val);
    }
    return ok;
}

}

