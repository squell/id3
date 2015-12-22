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

#if defined(__FreeBSD__) && (__FreeBSD__ < 5) \
 || defined(__DJGPP__) || defined(__BORLANDC__)
#    define to_upper toupper
#    define to_lower tolower
#    define is_(what, c) is##what(c)
#else
#    define to_upper towupper
#    define to_lower towlower
#    define is_(what, c) isw##what(c)
#endif

namespace {

enum style { as_is, name, lowr, split };

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

 // compress("    bla    bla  ") -> "bla bla"

void compress(wstring& s)
{
    wstring::iterator p = unique(s.begin(), s.end(), both_space());
    if(p != s.begin() && is_(space, p[-1])) --p;
    s.erase(p, s.end());
    if(s.length() > 0 && is_(space, s[0]))
        s.erase(s.begin());
}

 // noleadzero("(0300/0004)=0075") -> "300/4=75"

void noleadzero(wstring& s)
{
    const wchar_t zero[] = L"0";
    wstring::size_type p, q = 0;
    do {
        p = s.find_first_of(zero, q);
        q = s.find_first_not_of(zero, p);
        if(q == p)
            return;
        if(p == 0 || !is_(digit, s[p-1])) {
            s.erase(p, q-p);
            if(s.empty() || !is_(digit,s[p]))
                s.insert(p, zero);
            q = p+1;
        }
    } while(1);
}

 // capitalize("hElLo wOrLd") -> "Hello World"

void capitalize(wstring& s)
{
    bool new_w = true;
    for(wstring::iterator p = s.begin(); p != s.end(); ++p) {
        *p = new_w? to_upper(*p):to_lower(*p);
        new_w = is_(space, *p) || !is_(alnum, *p) && new_w;
    }
}

 // padcamels("ReformatAStringLikeThis") -> "Reformat A String Like This"

void padcamels(wstring& s)
{
    wstring::const_iterator p;
    bool word = false;
    wstring r;
    for(p = s.begin(); p != s.end(); r.push_back(*p++)) {
        if(is_(upper, *p) && word)
            r.push_back(' ');
        word = !is_(space, *p);
    }
    s.swap(r);
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

wchar_t codepoint(wstring::const_iterator& p, wstring::const_iterator const q, int digits)
{
    long val = 0;
    for( ; digits-- && p < q; ++p) {
        wchar_t c = to_upper(*p);
        if(is_(digit, c)) c -= '0';
        else if(c >= 'A' && c <= 'F') c -= 'A'-10;
        else break;
        val = val << 4 | (c&0xF);
    }
    return val;
}

} // end of anon. namespace

namespace stredit {

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
            case 'u':  c = codepoint(p, format.end(), 4); break;
            case 'U':  c = codepoint(p, format.end(), 8); break;
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
    unsigned num_pad = 0;

    for(++p; p != end; ) {
        switch(wchar_t c = *p++) {
        case '_': raw  = true;      continue;
        case '+': caps = name;      continue;
        case '-': caps = lowr;      continue;
        case '*': caps = split;     continue;
        case '#': ++num_pad;        continue;
        case prefix:
            return conv<wchar_t>(1, prefix);

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
            if(!subst.good()) for(unsigned i = 0; i < alt.size(); ++i)
                if(result tmp = edit(alt[i], true)) {
                    subst = tmp;
                    break;
                }
            wstring s = conv<wchar_t>(subst);
            if(!raw) {                          // remove gunk
                replace_if(s.begin(), s.end(), filtered_char(), ' ');
                compress(s);
            }
            if(caps == split)
                padcamels(s);
            if(caps == name)
                capitalize(s);
            if(caps == lowr)
                transform(s.begin(), s.end(), s.begin(), char_to_lower());
            if(num_pad > 0)
                noleadzero(s);
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

