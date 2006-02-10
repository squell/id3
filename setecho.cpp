#include <cstdio>
#include "setecho.h"

/*

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using tag::write::echo;

bool echo::vmodify(const char* fname, const function& edit) const
{
    return log( edit(fmt).c_str() ), true;
}

void echo::log(const char* msg) const
{
    std::puts(msg);
}

