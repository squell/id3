#include <string>
#include <cctype>
#include <algorithm>
#include "sedit.h"

#include <cstdio>

/*

  (c) 2003,2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

 // band-aid. the notice will be removed in the future too! ;)

struct sedit_deprecation_warning {
    bool flag1;
    bool flag2;
   ~sedit_deprecation_warning()
    {   if(flag1)
            printf("id3: note: %%c modifier will be removed in the future, "
                   "use %%+ instead.\n");
        if(flag2)
            printf("id3: note: %%n token will be removed in the future, "
                   "use %%, instead.\n");
    };
} sedit_depr = { false, false };

 // some predicates and transformers to feed to STL algo's

namespace {
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

 // remove extraneous spaces in a string

void compress(string& s)
{
    string::reverse_iterator t( unique(s.begin(), s.end(), both_space()) );
    if(t != s.rend() && isspace(*t)) ++t;
    s.erase(t.base(), s.end());
    if(s.length() > 0 && isspace(s[0]))
        s.erase(s.begin());
}

/* ====================================================== */

string svar::edit(string s, const base_container& v)
{
    enum style { as_is, name, lowr };

    string::size_type pos = 0;

    while( (pos=s.find(VAR, pos)) != string::npos ) {
        bool  raw  = false;
        style caps = as_is;
        int n = 1;
        while( pos+n < s.length() ) {
            switch( char c = toupper(s[pos+n]) ) {
            default:
                s.erase(pos, n);
                break;
            case '@':
            case ':': s.erase(pos, 1); s[pos++] = '\0'; break;
            case VAR: s.erase(pos, 1);   pos++;         break; // "%%" -> "%"
            case 'N': sedit_depr.flag2 = true;              // TO BE REMOVED
            case ',': s[pos++] = '\r'; s[pos++] = '\n'; break;

            case '_': raw  = true; ++n; continue;
            case 'C': sedit_depr.flag1 = true;              // TO BE REMOVED
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
    return s;
}

