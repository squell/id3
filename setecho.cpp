#include <cstdio>
#include <string>
#include "setecho.h"
#include "charconv.h"

/*

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

namespace set_tag {

bool echo::vmodify(const char* fname, const subst& v) const
{
    if(fmt)
        log( edit(fmt, v) );
    return fmt;
}

void echo::log(const cvtstring& msg) const
{
    std::puts(msg.local().c_str());
}

}

