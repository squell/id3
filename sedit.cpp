#include <string>
#include <cctype>
#include <algorithm>
#include "sedit.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

namespace {

 // some predicates and transformers to feed to STL algo's

    struct both_space {
        bool operator()(char a, char b)
        { return isspace(a) && isspace(b); }
    };

    struct control_char {
        bool operator()(char c)
        { return iscntrl(c); }
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

    void compress(string& s)
    {
        string::reverse_iterator t( unique(s.begin(), s.end(), both_space()) );
        if(t != s.rend() && isspace(*t)) ++t;
        s.erase(t.base(), s.end());
        if(s.length() > 0 && isspace(s[0]))
            s.erase(s.begin());
    }

 // choose capitalization

    enum style { as_is, name, lowr };

    string stylize(string s, style caps)
    {
        switch(caps) {
        case name:
            return capitalize(s);
        case lowr:
            transform(s.begin(), s.end(), s.begin(), to_lower());
        }
        return s;
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

cvtstring string_parm::edit(const cvtstring& fmt, const subst& var, const char* fallback)
{
    const cvtstring::xlat conv = &cvtstring::latin1;

    string::size_type pos = 0;
    string s = (fmt.*conv)();

    while( (pos=s.find(VAR, pos)) != string::npos ) {
        bool   raw  = false;
        style  caps = as_is;
        int    npad = 1;
        string alt  = fallback;

        int n = 1;
        while( pos+n < s.length() ) {
            cvtstring svar;

            switch( char c = s[pos + (n++)] ) {
            case VAR: s.replace(pos++, n, 1, VAR ); break;   // "%%" -> "%"
            case ',': s.replace(pos, n, "\r\n", 2); pos += 2; break;

            case '_': raw  = true; continue;
            case '+': caps = name; continue;
            case '-': caps = lowr; continue;
            case '#': ++npad;      continue;
            case '|': {                                        // alt string
                     int t = s.find('|',pos+n);
                     if(t == string::npos) {
                         s.replace(pos++, n, 1, '|');
                         break;
                     }
                     n  += pos;
                     alt = s.substr(n, t-n);
                     n   = t+1 - pos;
                     continue;
                }
            case '0': if(!ZERO_BASED) c += 10;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': if(!ZERO_BASED) --c;
                svar = var.numeric(c-'0');
                goto substitute;
            default :
                if(!isalpha(c)) {
                    s.replace(pos++, n, 1, '?');
                    break;
                } else
                    svar = var.alpha(c);
            substitute:
                if(svar.empty())
                    svar = edit(alt, var);
                string tmp = stylize((svar.*conv)(), caps);
                if(!raw) {                                     // remove gunk
                    replace_if(tmp.begin(), tmp.end(), control_char(), ' ');
                    replace(tmp.begin(), tmp.end(), '_', ' ');
                    compress(tmp);
                }
                if(npad > 1 && tmp.length() < npad)
                    tmp.insert(string::size_type(0),npad-tmp.length(), '0');
                s.replace(pos, n, tmp);
                pos += tmp.length();
            }

            break;         // turn switch-breaks into while-breaks
        }

    }

    return cvtstring::latin1(s);
}

