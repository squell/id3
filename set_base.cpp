/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#include <vector>
#include <algorithm>    ////////////////////
#include "set_base.h"

using namespace std;

namespace set_tag {

namespace {
    typedef vector<single_tag*>::const_iterator iter;
}

 /* This class does NOT delegate the free form methods. This is simply
    because all tag formats use different naming conventions, so it would
    be really pointless anyhow. */

handler& combined_tag::clear()
{
    for(iter p = tags.begin(); p != tags.end(); )
        (*p++)->clear();
    return *this;
}

handler& combined_tag::set(ID3field field, const char* data)
{
    for(iter p = tags.begin(); p != tags.end(); )
        (*p++)->set(field, data);
    return *this;
}

handler& combined_tag::active(bool state)
{
    for(iter p = tags.begin(); p != tags.end(); )
        (*p++)->active(state);
    return *this;
}

 /* This function logically OR's all active() states together */

bool combined_tag::active() const
{
    for(iter p = tags.begin(); p != tags.end(); )
        if((*p++)->active())
            return true;
    return false;
}

 /* implementation of combined vmodify()
    - obeys the vmodify restrictions of set_base.h */

bool combined_tag::vmodify(const char* fn, const base_container& val) const
{
    bool e = false;
    for(iter p = tags.begin(); p != tags.end(); ++p) {
        const single_tag* sub = *p;
        if(sub->active()) {
            if( !sub->vmodify(fn, val) ) {
                if(e) throw failure("partial tag written: ", fn);
                else  return false;
            }
            e = true;
        }
    }
    return e;
}

}

