#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>
#include "charconv.h"
#include "sedit.h"
#include "set_base.h"
#include "mass_tag.h"

/*

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;
using tag::ID3field;

namespace fileexp {

 // defining variable mapping characters <-> ID3field

ID3field mass_tag::field(wchar_t c)
{
     switch(c) {
     case 't': return tag::title;
     case 'a': return tag::artist;
     case 'l': return tag::album;
     case 'y': return tag::year;
     case 'c': return tag::cmnt;
     case 'n': return tag::track;
     case 'g': return tag::genre;
     case 'A': return tag::album;                    // compat.
     case 'T': return tag::track;
     default : return tag::FIELD_MAX;
     }
}

string mass_tag::var(int i)
{
    const char tab[] = "talycng";
    if(i < tag::FIELD_MAX)
        return string(1,'%') += tab[i];
    else
        return string();
}

namespace {

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

    unsigned long numfiles = 0;

    class substvars : public stredit::format {
    public:
        virtual result var(ptr& p, ptr) const;

        substvars(const char* fn,
                  const fileexp::record& rec,
                  const tag::reader& info,
                  unsigned long& x)
        : tag_data(0), num(0),
          tag(&info), filename(fn), filerec(&rec), cnt(&x) { }
       ~substvars()
        { delete tag_data; }

    private:
        mutable const tag::metadata* tag_data;
        mutable unsigned long int    num;

        const tag::reader*     const tag;
        const char*            const filename;
        const fileexp::record* const filerec;
        unsigned long int*     const cnt;

        substvars(const substvars&);       // don't copy
    };

    substvars::result substvars::var(ptr& p, ptr end) const
    {
        static const result empty(conv<latin1>("<empty>"), false);
        switch(wchar_t c = *p++) {
        case '?':
            return false;
        case 'x':
            if(!num) num = (*cnt)++;
            char buf[11];
            int n; n = sprintf(buf, "%lu", num & 0xFFFFu);   // to jump past
            return conv<latin1>(buf, n>0? n : 0);
        case 'X':
            n = sprintf(buf, "%lu", numfiles & 0xFFFFu);
            return conv<latin1>(buf, n>0? n : 0);
        case 'p':
            return conv<local>(filerec->path, filename - filerec->path);
        case 'f':
            return conv<local>(filename);
        case '0': c += 10;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            c -= '1';
            if(c >= filerec->var.size()) {
                static char error[] = "variable index out of range: %_";
                error[sizeof error-2] = (c+1)%10 + '0', throw out_of_range(error);
            }
            return conv<local>(filerec->var[c]);
	case '{': {
	    ptr q = std::find(p, end, '}');
	    if(p == end) {
                throw out_of_range("missing } in variable");
	    }
	    const string key = conv<wchar_t>(wstring(p, q)).str<latin1>();
	    p = q+1;

            if(!tag_data) tag_data = tag->read(filerec->path);
	    typedef tag::metadata::array info;
	    const info frames = tag_data->listing();
	    for(info::const_iterator rec = frames.begin(); rec != frames.end(); rec++) {
		if(key == rec->first.substr(0,max(string::size_type(3),key.length())))
		    return rec->second;
	    }
	    return conv<>();
	}

        default:
            ID3field i; i = mass_tag::field(c);
            if(i >= tag::FIELD_MAX) {
                static char error[] = "unknown variable: %_";
                error[sizeof error-2] = c, throw out_of_range(error);
            }
            if(!tag_data) tag_data = tag->read(filerec->path);
            const result& tmp = (*tag_data)[i];
            return tmp.empty()? empty : tmp;
        };
    }

}

unsigned long int mass_tag::total()
{
    return numfiles;
}

 // implementation of fileexp::find using tag objects

bool mass_tag::dir(const fileexp::record& d)
{
    return (counter = 1);
}

bool mass_tag::file(const char* name, const fileexp::record& f)
{
    substvars vars(name, f, tag_reader, counter);
    ++numfiles;

    return tag_writer.modify(f.path, vars);
}

} // namespace

