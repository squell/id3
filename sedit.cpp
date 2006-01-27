#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cwctype>
#include "sedit.h"

using namespace std;
using namespace charset;

#ifdef __DJGPP__                                 // crappy wide char support
#    define towupper toupper
#    define towlower tolower
#    define iswalnum isalnum
#    define iswcntrl iscntrl
#    define iswspace isspace
#endif

namespace stredit {

enum style { as_is, name, lowr, camel };

struct filtered_char {                           // filter low-ascii
    bool operator()(unsigned char c)
    { return c == '_' || iswcntrl(c); }
};

struct both_space {                              // filter ascii space
    bool operator()(char a, char b)
    { return iswspace(a) && iswspace(b); }
};

struct to_lower {
    char operator()(wchar_t c)
    { return towlower(c); }
};

 // compress("    bla    bla  ", 4) -> "bla bla"

void compress(wstring& s)
{
    wstring::iterator p = unique(s.begin(), s.end(), both_space());
    if(p != s.begin() && iswspace(p[-1])) --p;
    s.erase(p, s.end());
    if(s.length() > 0 && iswspace(s[0]))
        s.erase(s.begin());
}

 // capitalize("hElLo wOrLd", 4) -> "Hello World"

void capitalize(wstring& s)
{
    bool new_w = true;
    for(wstring::iterator p = s.begin(); p != s.end(); ++p) {
        *p = new_w? towupper(*p):towlower(*p);
        new_w = iswspace(*p) || !iswalnum(*p) && new_w;
    }
}

 // padnumeric("(300/4)=75", 4) -> "0300/0004=0075"

void padnumeric(wstring& s, unsigned pad)
{
    const wchar_t digits[] = L"0123456789";
    wstring::size_type p, q = 0;
    do {
        p = s.find_first_of    (digits, q);
        q = s.find_first_not_of(digits, p);
        wstring::size_type l = ((q==wstring::npos)? s.length() : q) - p;
        if(q == p)
             return;
        if(l < pad) {
             s.insert(p, pad-l, '0');
             l = pad;
        }
        q = p + l;
    } while(1);
}

function::result format::edit(const wstring& format, bool atomic) const
{
    conv<wchar_t> build;
    build.str().reserve(format.length());
    int validity = -true;

    for(ptr p = format.begin(); p < format.end(); ) {
        switch(wchar_t c = *p++) {
        case '\\':                              // leaves trailing slashes
            if(p != format.end()) switch(c = *p++) {
            case '\\': c = '\\'; break;
            case 'a':  c = '\a'; break;
            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;    /* ?append carriage returns? */
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;
            case 'v':  c = '\v'; break;
            }
        default:
            build += c;
            break;
        case '%':
            result subst = code(--p, format.end());
            build += subst;
            if(!subst) {
                validity = validity > false;
                continue;
            }
        }
        if(!atomic) validity = true;
    }
    return result(build, validity);
}

function::result format::code(ptr& p, ptr end) const
{
    vector<wstring> alt;
    style caps       = as_is;
    bool raw         = false;
    unsigned num_pad = 1;

    for(++p; p != end; ) {
        switch(wchar_t c = *p++) {
        case '_': raw  = true;      continue;
        case '+': caps = name;      continue;
        case '-': caps = lowr;      continue;
        case '#': ++num_pad;        continue;
        case '%':
            return conv<wchar_t>(1, '%');
        case ',':                               // deprecated new line macro
            struct _deprecated {
                ~_deprecated()
                { }
            } static _warning;
            return conv<wchar_t>(1, '\n');

        case '|': {
            ptr q = matching(p-1, end);
            if(alt.size() < 25)                 // artificial limit
                alt.push_back(wstring(p, q));
            if(q == end) break;
            p = ++q;
            continue;
          }

        default :
            result subst = var(--p, end);
            for(int i = 0; !subst && i < alt.size(); ++i) {
                subst = edit(alt[i], true);
            }
            wstring s = conv<wchar_t>(subst).str();
            if(!raw) {                          // remove gunk
                replace_if(s.begin(), s.end(), filtered_char(), ' ');
                compress(s);
            }
            if(caps == name)
                capitalize(s); else
            if(caps == lowr)
                transform(s.begin(), s.end(), s.begin(), to_lower());
            padnumeric(s, num_pad);
            return result(conv<wchar_t>(s), subst.good());
        }
        break;
    }
    return false;
}

format::ptr format::matching(ptr p, ptr end) const
{
    unsigned nesting = 1, one = 1;
    wchar_t delim = *p++;
    while(p != end) {
        switch(wchar_t c = *p++) {
        case '%' :                              // start of nesting?
            one = -one; break;
        case '\\':                              // ignore escaped char
            if(p == end) break;
            ++p;
        default  :
            if(c == delim && (nesting-=one) == 0) return --p;
            one = 1;
        }
    }
    return p;
}

} // end namespace

