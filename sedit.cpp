#include <string>
#include <cctype>
#include <algorithm>
#include "charconv.h"
#include "sedit.h"

#include <cstdio>                                 // for bandaid

/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

namespace {

 // some predicates and transformers to feed to STL algo's

    struct both_space {
        bool operator()(char a, char b)
        { return isspace(a) && isspace(b); }
    };

    struct to_lower {
        char operator()(char c)
        { return tolower(c); }
    };

    struct not_space {
        bool operator()(char c)
        { return !isspace(c); }
    };

 // remove extraneous spaces in a string

    inline void compress(string& s)
    {
        string::reverse_iterator t( unique(s.begin(), s.end(), both_space()) );
        if(t != s.rend() && isspace(*t)) ++t;
        s.erase(t.base(), s.end());
        if(s.length() > 0 && isspace(s[0]))
            s.erase(s.begin());
    }
}

 // Capitalize A Text-string Like This.

string capitalize(string s)
{
    bool new_w = true;
    for(string::iterator p = s.begin(); p != s.end(); p++) {
        *p = new_w? toupper(*p):tolower(*p);
        new_w = isspace(*p) || !isalnum(*p) && new_w;
    }
    return s;
}

/* ====================================================== */

 // this is just a placeholder until sedit is fully wide-character aware

inline void convert_to_latin1(string& s)
{
     string tmp = latin1::conv(s);
     s.swap(tmp);
}

/* ====================================================== */

string string_parm::edit(string s, const base_container& v)
{
    enum style { as_is, name, lowr };

    string::size_type pos = 0;

    while( (pos=s.find(VAR, pos)) != string::npos ) {
        bool  raw  = false;
        style caps = as_is;
        int n = 1;
        while( pos+n < s.length() ) {
            switch( char c = s[pos+n] ) {
            default:
                s.erase(pos, n);
                break;
            case '@':
            case ':': s.erase(pos, 1); s[pos++] = '\0'; break;
            case VAR: s.erase(pos, 1);   pos++;         break; // "%%" -> "%"
            case ',': s[pos++] = '\r'; s[pos++] = '\n'; break;

            case '_': raw  = true; ++n; continue;
            case '+': caps = name; ++n; continue;
            case '-': caps = lowr; ++n; continue;

            case '0': if(!ZERO_BASED) c += 10;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                string tmp = v[c-'1' +ZERO_BASED];
                if(!raw) {                                  // remove gunk
                    replace(tmp.begin(), tmp.end(), '_', ' ');
                    compress(tmp);
                }
                switch(caps) {
                case name:
                    tmp = capitalize(tmp); break;
                case lowr:
                    transform(tmp.begin(), tmp.end(), tmp.begin(), to_lower());
                }
                s.replace(pos, n+1, tmp);
                pos += tmp.length();
                break;
            }
            break;         // turn switch-breaks into while-breaks
        }
    }

    convert_to_latin1(s);
    return s;
}

