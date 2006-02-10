#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>
#include <cctype>
#include "sedit.h"

/*

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;
using namespace charset;

 /*
   <wctype.h> was added in the 1994 Amd. to C; __STDC_VERSION__ >= 199409L
   It is therefore part of the C++ standard, but support varies; autoconf?
 */

#if defined(__FreeBSD__) && (__FreeBSD__ <= 5) \
 || defined(__DJGPP__) || defined(__BORLANDC__)
#    define to_upper toupper
#    define to_lower tolower
#    define is_(what, c) is##what(c)
#else
#    define to_upper towupper
#    define to_lower towlower
#    define is_(what, c) isw##what(c)
#endif

extern void deprecated(const char*);

namespace stredit {

enum style { as_is, name, lowr, camel };

struct filtered_char {                           // filter low-ascii
    bool operator()(wchar_t c)
    { return c == '_' || is_(cntrl, c); }
};

struct both_space {                              // filter ascii space
    bool operator()(wchar_t a, wchar_t b)
    { return is_(space, a) && is_(space, b); }
};

struct char_to_lower {
    wchar_t operator()(wchar_t c)
    { return to_lower(c); }
};

 // compress("    bla    bla  ", 4) -> "bla bla"

void compress(wstring& s)
{
    wstring::iterator p = unique(s.begin(), s.end(), both_space());
    if(p != s.begin() && is_(space, p[-1])) --p;
    s.erase(p, s.end());
    if(s.length() > 0 && is_(space, s[0]))
        s.erase(s.begin());
}

 // capitalize("hElLo wOrLd", 4) -> "Hello World"

void capitalize(wstring& s)
{
    bool new_w = true;
    for(wstring::iterator p = s.begin(); p != s.end(); ++p) {
        *p = new_w? to_upper(*p):to_lower(*p);
        new_w = is_(space, *p) || !is_(alnum, *p) && new_w;
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
    build.reserve(format.length());
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
        case prefix:
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
        case prefix:
            return conv<wchar_t>(1, prefix);
/* */   case ',':                               // deprecated new line macro
            { struct _deprecated {
               _deprecated() { }
               ~_deprecated()
                { deprecated("`%,' will be removed, use `\\n' or `\\n\\r' instead"); }
              } static _warning; }
/* */       return conv<wchar_t>(1, '\n');

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
            if(!subst.good()) for(int i = 0; i < alt.size(); ++i)
                if(result tmp = edit(alt[i], true)) {
                    subst = tmp;
                    break;
                }
            wstring s = conv<wchar_t>(subst).str();
            if(!raw) {                          // remove gunk
                replace_if(s.begin(), s.end(), filtered_char(), ' ');
                compress(s);
            }
            if(caps == name)
                capitalize(s); else
            if(caps == lowr)
                transform(s.begin(), s.end(), s.begin(), char_to_lower());
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
        case prefix:                            // start of nesting?
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

