/*

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

#include "setgroup.h"
#include <numeric>
#include <functional>

using namespace std;

typedef int concreteness_check[ sizeof tag::combined() ];

namespace tag {

combined& combined::rewrite(bool t)
{
    for_each(begin(), end(), bind2nd(mem_fun(&handler::rewrite), t));
    return *this;
}

combined& combined::create(bool t)
{
    for_each(begin(), end(), bind2nd(mem_fun(&handler::create), t));
    return *this;
}

combined& combined::set(ID3field i, std::string m)
{
    for(iterator p = begin(); p != end(); ) (*p++)->set(i,m);
    return *this;
}

bool combined::from(const char* fn)
{
    bool result = false;
    for(iterator p = begin(); p != end(); ) result |= (*p++)->from(fn);
    return result;
}

bool combined::vmodify(const char* fn, const function& f) const
{
    bool result = false;
    for(const_iterator p = begin(); p != end(); )
        result |= (*p++)->modify(fn, f);
    return result;
}

}

