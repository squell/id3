/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#include <vector>
#include <string>
#include "set_base.h"

using namespace std;

namespace set_tag {

 /* Internal implementation details.
    - seperated from main header for ease of compilation */

struct combined::internal {
    typedef pair<ID3field, string>           command;
    typedef vector<handler*>::const_iterator iterator;

    vector<handler*> tags;
    vector<command>  args;

    static const command clear;                 // sentinel value

    iterator begin() const { return tags.begin(); }
    iterator end()   const { return tags.end();   }

    void transfer();
};

const combined::internal::command combined::internal::clear(FIELDS, "");

combined::combined()
: impl(new internal)
{
}

combined::~combined()
{
    delete impl;
}

 /* This class does NOT delegate the free form methods. This is simply
    because all tag formats use different naming conventions, so it would
    be really pointless anyhow. */

combined& combined::set(ID3field i, string data)
{
    if(i < FIELDS)
        impl->args.push_back( internal::command(i, data) );
    return *this;
}

combined& combined::clear()
{
    impl->args.push_back( internal::clear );
    return *this;
}

combined& combined::delegate(handler& h)
{
    for(internal::iterator p = impl->begin(); p != impl->end(); ++p) {
        if(*p == &h) return *this;
    }
    impl->tags.push_back(&h);
    return *this;
}

 /* Note that if delegates are added after a transfer, the original
    set functions will have been lost on it. This can be remedied,
    but it would cost more code size (and complexity), and have little
    practical value. */

void combined::internal::transfer()
{
    for(vector<command>::iterator a = args.begin(); a != args.end(); ++a) {
        for(iterator p = begin(); p != end(); ++p)
            if(*a == clear)
                (*p)->clear();
            else
                (*p)->set(a->first, a->second);
    }
    args.clear();
}

 /* Implementation of combined vmodify()
    - obeys the vmodify restrictions of set_base.h */

bool combined::vmodify(const char* fn, const subst& val) const
{
    if(! impl->args.empty() ) {
        impl->transfer();
    }

    bool ok = true;                      // process delegates
    for(internal::iterator p = impl->begin(); p != impl->end(); ++p) {
        ok &= (*p)->vmodify(fn, val);
    }
    return ok;
}

}

