#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include "dumptag.h"

/*

  serializing tag info to/from human-readable text

  copyright (c) 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

namespace tag {

using namespace std;

#define DIRECTIVE "#"
#define COMPACT false

void output(metadata::array::const_iterator begin, metadata::array::const_iterator end, FILE* out)
{
    while(begin < end) {
        const pair<string,string> line = *begin++;
        const char* key   = line.first.c_str();
        const char* value = line.second.c_str();
        const char* lnbrk = strchr(value, '\n');
        if(COMPACT && !lnbrk && !strchr(key, ':')) {
            // emit one a single line
            fprintf(out, "{%s}\t%s\n", key, value);
        } else {
            // emit as header-paragraph
            fprintf(out, "{%s}\n", key);
            while(lnbrk) {
                int const span = (lnbrk - value) & 0xFFFF;
                fprintf(out, "\t%.*s\n", (int)span, value);
                value = lnbrk+1;
                lnbrk = strchr(value, '\n');
            }
            fprintf(out, "\t%s\n", value);
        }
    }
}

void output(combined<reader> const& tags, const char* filename, FILE* out)
{
    fprintf(out, DIRECTIVE"file\t%s\n", filename);
    combined<reader>::const_iterator p = tags.begin();
    while( p != tags.end() ) {
        std::auto_ptr<metadata> info( (*p++)->read(filename) );
        if(info.get() && *info) {
            metadata::array list = info->listing();
            const pair<string,string> hdr = list[0];
            fprintf(out, DIRECTIVE"tag\t%s %s\n", hdr.first.c_str(), hdr.second.c_str());
            output(list.begin()+1, list.end(), out);
        }
    }
    fprintf(out, "\n");
    if(ferror(out))
        throw failure("could not emit tag data");
}

}
