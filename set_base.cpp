/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#include "set_base.h"

using namespace std;

namespace set_tag {

 /* This class does NOT delegate the free form methods. This is simply
    because all tag formats use different naming conventions, so it would
    be really pointless anyhow. */

namespace {
    typedef vector<handler*>::const_iterator reg_iter;
}

group& group::delegate(handler& h)
{
    for(reg_iter p = reg.begin(); p != reg.end(); ++p) {
        if(*p == &h) return *this;
    }
    reg.push_back(&h);
    return *this;
}

 /* Implementation of group vmodify()
    - obeys the vmodify restrictions of set_base.h */

bool group::vmodify(const char* fn, const subst& val) const
{
    for(int n = 0; n < FIELDS; ++n)
        if(const string* txt = data.update[n]) {
            for(reg_iter p = reg.begin(); p != reg.end(); ) {
                (*p++)->set(ID3field(n), *txt);
            }
            data.update[n] = 0;
        }

    if(data.cleared) {
        for(reg_iter p = reg.begin(); p != reg.end(); (*p++)->clear());
        data.cleared = false;
    }

    bool ok = true;                      // process delegates
    for(reg_iter p = reg.begin(); p != reg.end(); ) {
        ok &= (*p++)->vmodify(fn, val);
    }
    return ok;
}

}

