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

    struct filtered_char {
        bool operator()(char c)
        { return c == '_' || iscntrl(c); }
    };

    struct to_lower {
        char operator()(char c)
        { return tolower(c); }
    };

 // remove extraneous spaces in a string

    void compress(string& s)
    {
        string::iterator p = unique(s.begin(), s.end(), both_space());
        if(p != s.begin() && isspace(p[-1])) --p;
        s.erase(p, s.end());
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

#ifdef GROUP_EXT
    string::size_type findclose(const string& s, string::size_type i)
    {
        int n = 1;
        while(n && i < s.length())
            switch(s[i++]) {
            case '>': --n; break;
            case '<': ++n; break;
            }
        return n==0? --i : string::npos;
    }

#endif
}

 // Capitalize A Text-string Like This.

string capitalize(string s)
{
    bool new_w = true;
    for(string::iterator p = s.begin(); p != s.end(); ++p) {
        *p = new_w? toupper(*p):tolower(*p);
        new_w = isspace(*p) || !isalnum(*p) && new_w;
    }
    return s;
}

 // padnumeric("(300/4)=75", 4) -> "0300/0004=0075"

string padnumeric(string s, int pad)
{
    const char digits[] = "0123456789";
    string::size_type p, q = 0;
    do {
        p = s.find_first_of    (digits, q);
        q = s.find_first_not_of(digits, p);
        string::size_type l = ((q==string::npos)? s.length() : q) - p;
        if(q == p)
             return s;
        if(l < pad) {
             s.insert(p, pad-l, '0');
             l = pad;
        }
        q = p + l;
    } while(1);
}

/* ====================================================== */

cvtstring string_parm::edit(const cvtstring& fmt, const subst& var, const char* fallback, bool atomic)
{
    const cvtstring::xlat conv = &cvtstring::latin1;

    string::size_type pos = 0;
    string s = (fmt.*conv)();
    bool err = false;             // keeps track if all substitutions worked

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
                     int t = s.find('|', pos+n);
                     if(t == string::npos) {
                         s.replace(pos++, n, 1, '|');
                         break;
                     }
                     n  += pos;
                     alt = s.substr(n, t-n);
                     n   = t+1 - pos;
                     continue;
                }
#ifdef GROUP_EXT
            case '<': {                                        // grouping
                     int t = findclose(s, pos+n);
                     if(t == string::npos) {
                         s.replace(pos++, n, 1, '|');
                         break;
                     }
                     n  += pos;
                     svar= edit(s.substr(n, t-n),var,fallback,true);
                     alt = "";
                     raw = true;
                     n   = t+1 - pos;
                     goto substitute;
                }
#endif
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
                if(svar.empty()) {
                    svar = edit(alt, var);
                    err = raw = true;
                }
                string tmp = stylize((svar.*conv)(), caps);
                if(!raw) {                                     // remove gunk
                    replace_if(tmp.begin(), tmp.end(), filtered_char(), ' ');
                    compress(tmp);
                }
                s.replace(pos, n, padnumeric(tmp, npad));
                pos += tmp.length();
            }

            break;         // turn switch-breaks into while-breaks
        }

    }

    return !(err&&atomic)? cvtstring::latin1(s) : cvtstring();
}

