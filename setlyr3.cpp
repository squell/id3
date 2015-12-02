#include <string>
#include <map>
#include "charconv.h"
#include "lyrics3.h"
#include "getlyr3.h"
#include "setlyr3.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;

using tag::write::Lyrics3;
using tag::ID3field;

namespace {
    typedef int concreteness_check[ sizeof Lyrics3() ];
}

#pragma GCC diagnostic ignored "-Wswitch"

Lyrics3& Lyrics3::set(ID3field i, string m)
{
    const char (*ptr)[4] = read::Lyrics3::field_name;
    switch(i) {
    case tag::album:  ++ptr;
    case tag::artist: ++ptr;
    case tag::title:
        set(*ptr, m);
    }
    return *this;
}

bool Lyrics3::from(const char* fn)
{
    null_tag = lyrics3::read(fn).str();
    return null_tag.size();
}

bool Lyrics3::set(string field, string s)
{
    if(lyrics3::field(field, "0").size() != 0) {
        mod[field] = s;
        return true;
    }
    return false;
}

bool Lyrics3::rm(string field)
{
    mod[field].erase();
    return true;
}

/* ===================================== */

tag::metadata* Lyrics3::read(const char* fn) const
{
    return new read::Lyrics3(fn);
}

bool Lyrics3::vmodify(const char* fn, const function& edit) const
{
    typedef map<string, string> db;
    db table(mod);

    lyrics3::info tag, src = lyrics3::read(fn);

    if(src.size() != 0 || gen) {
        if(fresh) src = lyrics3::cast(null_tag);
    } else
        return true;

    for(long n, i = 0; n=lyrics3::find_next(src,i); i = n) {
        db::iterator p = table.find( src.id(i) );
        if(p == table.end())
            tag += lyrics3::field(src.id(i), src.content(i, n));
        else {
            if(!p->second.empty()) {            // else: erase frames
                if(function::result s = edit(p->second))
                    tag += lyrics3::field(p->first, s.str<charset::latin1>());
                else
                    tag += lyrics3::field(p->first, src.content(i, n));
                table.erase(p);
            }
        }
    }

    for(db::iterator p = table.begin(); p != table.end(); ++p) {
        if(!p->second.empty()) {
            if(function::result s = edit(p->second))
                tag += lyrics3::field(p->first, s.str<charset::latin1>());
        }
    }

    int status = lyrics3::write(fn, tag);
    if(status < 0)
        throw tag::failure("error writing LYRICS to ", fn);
    return status == 0;
}
