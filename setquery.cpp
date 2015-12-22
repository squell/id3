#include <cstdio>
#include "setquery.h"

/*

  copyright (c) 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using tag::write::query;

typedef int concreteness_check[ sizeof query() ];

bool query::vmodify(const char* fname, const function& edit) const
{
    return log( std::string(edit(fmt)).c_str() ), true;
}

void query::log(const char* msg) const
{
    std::puts(msg);
}

