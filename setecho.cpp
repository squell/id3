#include <cstdio>
#include "setecho.h"

/*

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

namespace set_tag {

bool echo::vmodify(const char* fname, const subst& v) const
{
    return log( edit(fmt, v) ), true;
}

void echo::log(charset::conv<charset::local> msg) const
{
    std::puts(msg.c_str());
}

}

