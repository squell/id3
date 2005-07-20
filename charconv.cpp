#include <cstdlib>
#include <cstring>
#include <clocale>
#include "charconv.h"

/*

  character conversion (user locale -> latin1) (portable)

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

struct cvtstring::initialize {
    initialize();
    const char* const loc;                     // holds result of setlocale
} cvtstring::instance;

namespace {
    using namespace std;

#ifdef __STDC_ISO_10646__                      // wchar_t always unicode?
    struct wide_input {
        size_t      len;
        const char* end;
        bool        e;

        wide_input(const char* ptr, size_t cnt)
        : len(cnt), end(ptr+cnt), e(false)
        { mbtowc(0, 0, 0); }                 // clear internal state

        bool operator>>(wchar_t& wc)
        {
            while(int n = mbtowc(&wc, &end[-len], len)) {
                if(n > 0) {
                    len -= n;
                    e = false;
                } else {
                    len --;
                    if(e) continue;
                    wc = 0xFFFD;
                    e = true;
                }
                return true;
            }
            return false;
        }
    };

    struct wide_output {
        string str;

        wide_output()
        { wctomb(0,0); }

        void operator<<(wchar_t wc)
        {
            char buf[MB_CUR_MAX];
            int  n = wctomb(buf, wc);
            if(n > 0) str.append(buf, n);
            else      str.push_back('?');
        }
    };

    string make_latin(const string& str)
    {
        wide_input widestr(str.c_str(), str.length());
        wchar_t    wc;
        string     out;

        while(widestr >> wc)
            out.push_back(wc<=0xFF ? wc : '?');

        return out;
    }

    string make_system(const string& str)
    {
        wide_output out;

        for(string::const_iterator p = str.begin(); p != str.end(); ++p)
            out << (*p & 0xFF);

        return out.str;
    }

#endif
    string fallback(const string& str)
    {
        return str;                             // simply assume Latin 1
    }

} // end of namespace

string (*cvtstring::conv_to_internal)(const string&) = fallback;
string (*cvtstring::conv_to_locale)  (const string&) = fallback;

const char* cvtstring::system_charset()
{
    return instance.loc;
}

#ifdef __STDC_ISO_10646__

cvtstring::initialize::initialize() : loc(setlocale(LC_CTYPE, ""))
{
    if(loc && strcmp(loc, "POSIX") != 0 && strcmp(loc, "C") != 0) {
        conv_to_internal = make_latin;
        conv_to_locale   = make_system;
    } else {
        conv_to_internal = fallback;
        conv_to_locale   = fallback;
    }
}

#else

cvtstring::initialize::initialize() : loc("C")
{ }

#endif

