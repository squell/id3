/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#include <vector>
#include <cstring>
#include "set_base.h"

using namespace std;

namespace set_tag {

namespace {
    typedef vector<handler*>::const_iterator iter;
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

}

