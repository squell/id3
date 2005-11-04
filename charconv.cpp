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

        wide_input(const string& s)
        : len(s.size()), end(s.data()+len), e(false)
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
        { str.reserve(256); wctomb(0,0); }

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
        wide_input widestr(str);
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

#if defined( __STDC_ISO_10646__ )

cvtstring::initialize::initialize() : loc(setlocale(LC_CTYPE, ""))
{
    if(loc && strcmp(loc, "POSIX") != 0 && strcmp(loc, "C") != 0) {
        conv_to_internal = make_latin;
        conv_to_locale   = make_system;
    } else {
        conv_to_internal = fallback;            // assume latin1 if POSIX
        conv_to_locale   = fallback;            // (so don't support EBCDIC;)
    }
}

#else

cvtstring::initialize::initialize() : loc("C")
{ }

#endif

 /*
   Notes:

   __STDC_ISO_10646__ is a C99 constant. If defined, wchar_t is
   guaranteed to be a coded representation of the Unicode set in all
   locales. This is bliss. glibc2.2 defines it, so this covers Linux.


   On Windows, you need to fight the jargon first;

   "Unicode" = UCS2 16bit chars
   "ANSI"    = "Windows codepage" (such as CP1252)
   "OEM"     = "DOS codepage"     (such as CP437, CP850, CP858)

   In true Microsoft fashion, ANSI and OEM are two different beasts, and so
   there are always *two* codepages active! You CAN use ANSI codepages on the
   Win32 commandline in NT/2K/XP, and also UTF8 ("codepage 65001"), but these
   will only display properly with a TrueType font.

   Commandlines are apparently converted to "ANSI" codepage before being
   passed. If you want the "Unicode" version, there's GetCommandLineW in
   windows.h.

   So; arguments a program get will be in "correct" ANSI codepage, but I/O
   (e.g., pipes, console output) will not be. Console output should be in OEM
   but file output should (probably) be in ANSI. File routines can be either
   OEM or ANSI style.

   Windows also has two locales for converting multibyte chars. The ISO C mb
   functions from stdlib.h and wchar.h listens to setlocale(), but most MS
   runtime functions listen to _setmbcp.

   Second, setlocale(LC_CTYPE, "") might get the active ANSI or OEM codepage!
   MinGW does the former, Borland C++ the latter.

   So the problem is not converting to Unicode - mbtowc does this! - but to
   actually select the proper locale.

   Related routines, without stupid MS typedefs;
    wchar_t* GetCommandLineW(void)
    char*    GetCommandLineA(void)
    bool     AreFileApisANSI(void)
    void     SetFileApisToOEM(void)
    void     SetFileApisToANSI(void)
    unsigned GetACP(void)
    unsigned GetOEMACP(void)
    unsigned GetConsoleOutputCP(void)  // why two ?
    unsigned GetConsoleCP(void)
    unsigned SetConsoleOutputCP(void)  // NT only! since 9x has no console
    unsigned SetConsoleCP(void)        // NT only! since 9x has no console

   I have had to read MSDN to get this info. I am utterly disgusted.

  */

