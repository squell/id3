#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <string>
#include "charconv.h"
#include "sedit.h"
#include "mass_tag.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;
using namespace charset;
using set_tag::ID3field;

namespace fileexp {

 // defining variable mapping characters <-> ID3field

ID3field mass_tag::field(char c)
{
     switch(c) {
     case 't': return set_tag::title;
     case 'a': return set_tag::artist;
     case 'l': return set_tag::album;
     case 'y': return set_tag::year;
     case 'c': return set_tag::cmnt;
     case 'n': return set_tag::track;
     case 'g': return set_tag::genre;
     default : return set_tag::FIELDS;
     }
}

string mass_tag::var(int i)
{
    const char tab[] = "talycng";
    if(i < set_tag::FIELDS)
        return string(1,'%') += tab[i];
    else
        return string();
}

namespace {

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

    class substvars : public stredit::format {
    public:
        virtual result var(ptr& p, ptr) const;

        substvars(const set_tag::provider& info, const fileexp::record& fn, unsigned x)
        : tag_data(0), tag(&info), filerec(&fn), num(x) { }
       ~substvars()
        { delete tag_data; }

    private:
        mutable const set_tag::reader* tag_data;

        const set_tag::provider* const tag;
        const fileexp::record*   const filerec;
        unsigned int             const num;

        substvars(const substvars&);       // don't copy
    };

    substvars::result substvars::var(ptr& p, ptr) const
    {
        static const result empty(conv<latin1>("<empty>"), false);
        switch(char c = *p++) {
        case '?':
            return false;
        case 'x':
            char buf[11];
            int n; n = sprintf(buf, "%u", num & 0xFFFFu);   // to jump past
            return conv<latin1>(buf, n>0? n : 0);
        case 'F':
            return conv<local>(filerec->path);
        case 'f':
            if(const char* p = strrchr(filerec->path,'/'))
                  return conv<local>(p+1);
              else
                  return conv<local>(filerec->path);
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
        default:
            ID3field i; i = mass_tag::field(c);
            if(i >= set_tag::FIELDS) {
                static char error[] = "unknown variable: %_";
                error[sizeof error-2] = c, throw out_of_range(error);
            }
            if(!tag_data) tag_data = tag->read(filerec->path);
            result tmp = (*tag_data)[i];
            return tmp? tmp : empty;
        };
    }

}

 // implementation of fileexp::find using set_tag objects

bool mass_tag::dir(const fileexp::record& d)
{
    return (counter = 1);
}

bool mass_tag::file(const char* name, const fileexp::record& f)
{
    substvars vars(tag_info, f, counter++);

    return tag_update.modify(f.path, vars);
}

} // namespace

