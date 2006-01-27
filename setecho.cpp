#include <cstdio>
#include "setecho.h"

/*

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

namespace set_tag {

bool echo::vmodify(const char* fname, const function& edit) const
{
    return log( edit(fmt).c_str() ), true;
}

void echo::log(const char* msg) const
{
    std::puts(msg);
}

}

