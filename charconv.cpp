#include <cstdlib>
#include <cstring>
#include <clocale>
#include <string>
#include "charconv.h"

/*

  character conversion (user locale -> latin1) (portable)

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

namespace latin1 {
    using namespace std;

    string (*conv)(const std::string&) = fallback;

#ifdef __STDC_ISO_10646__                      // wchar_t always unicode?
    struct initialize {
        initialize();
        const char* const loc;                 // holds result of setlocale
    } instance;

    const char* charset()
    {
        return instance.loc;
    }

    string make_latin(const string& str)
    {
        size_t      len = str.length();
        const char* end = str.c_str() + len;
        bool        e   = false;
        wchar_t     wc;
        string      out;

        mbtowc(0, 0, 0);                        // clear internal state
        while(int n = mbtowc(&wc, &end[-len], len)) {
            if(n > 0) {
                out.push_back(wc<=0xFF ? wc : '?');
                len -= n;
                e = false;
            } else {
                if(!e) out.push_back(0x81);
                len --;
                e = true;
            }
        }
        return out;
    }

    initialize::initialize() : loc(setlocale(LC_CTYPE, ""))
    {
        if(loc && strcmp(loc, "POSIX") != 0 && strcmp(loc, "C") != 0)
            conv = make_latin;
        else
            conv = fallback;
    }
#else

    const char* charset()
    {
        return "C";
    }
#endif

    string fallback(const string& str)
    {
        return str;                             // simply assume Latin 1
    }

} // end of namespace

