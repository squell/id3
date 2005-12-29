#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <string>
#include "charconv.h"
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

 // range checked, boxed, constrained vector

    class r_vector {
        const vector<string>& vec;
    public:
        r_vector(const vector<string>& v) : vec(v) { }
        inline const string& operator[](size_t i) const
        {
            if(i >= vec.size())
                throw out_of_range("variable index out of range");
            return vec[i];
        }
    };

 // variable mapping for substitution
 // - only reads tag data from file when actually requested

    class substvars {
    public:
        conv<local> operator[](char field) const;

        substvars(const set_tag::provider& info, const char* fn, unsigned x)
        : tag_data(0), tag(&info), filename(fn), num(x) { }
       ~substvars()
        { delete tag_data; }

    private:
        mutable const set_tag::reader* tag_data;

        const set_tag::provider* const tag;
        const char*              const filename;
        unsigned int             const num;

        substvars(const substvars&);       // don't copy
    };

    conv<local> substvars::operator[](char c) const
    {
        switch( c ) {
            ID3field i;
            char buf[11];
        default:
            i = mass_tag::field(c);
            if(i < set_tag::FIELDS) {
                if(!tag_data) tag_data = tag->read(filename);
                return (*tag_data)[i];
            }
            break;
        case 'x':
            int n; n = sprintf(buf, "%u", num & 0xFFFFu);   // to jump past
            return conv<latin1>(buf, n>0? n : 0);
        case 'F':
            return filename;
        case 'f':
            if(const char* p = strrchr(filename,'/'))
                  return p+1;
              else
                  return filename;
        };
        static char error[] = "unknown variable: %_";
        error[sizeof error-2] = c;
        throw out_of_range(error);
    }

}

 // implementation of fileexp::find using set_tag objects

bool mass_tag::dir(const fileexp::record& d)
{
    return (counter = 1);
}

bool mass_tag::file(const char* name, const fileexp::record& f)
{
    substvars letter_vars(tag_info, f.path, counter++);
    r_vector  number_vars(f.var);

    return tag_update.modify(f.path, number_vars, letter_vars);
}

} // namespace

