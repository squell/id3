/*

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

#include "setgroup.h"

using namespace std;

namespace tag {

bool combined::from(const char* fn)
{
    fn? basefn = fn : basefn = 0;
    return fn;
}

 /* This class does NOT delegate the free form methods. This is simply
    because all tag formats use different naming conventions, so it would
    be really pointless anyhow. */

 /* Implementation of group vmodify()
    - obeys the vmodify restrictions of set_base.h */

 /* This is wrong */

bool combined::vmodify(const char* fn, const function& val) const
{
    for(int n = 0; n < FIELD_MAX; ++n)
        if(const string* txt = data.update[n]) {
            for(const_iterator p = begin(); p != end(); ) {
                (*p++)->set(ID3field(n), *txt);
            }
            data.update[n] = 0;
        }

    if(data.cleared) {
        for(const_iterator p = begin(); p != end(); ) (*p++)->clear();
        data.cleared = false;
    }

    if(basefn) {
        for(const_iterator p = begin(); p != end(); ) (*p++)->from(basefn->c_str());
        basefn = 0;
    }

    bool ok = true;                      // process delegates
    for(const_iterator p = begin(); p != end(); ) {
        ok &= (*p++)->modify(fn, val);
    }
    return ok;
}

}

