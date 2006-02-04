#if defined(__WIN32__)
#  include <windows.h>
#  include <cstdio>
#endif
#include <cstddef>
#include <clocale>
#include <climits>
#if defined(__STDC_ISO_10646__) || defined(__WIN32__)
#  include <wchar.h>
#  define fallback(call) (0)
#elif defined(__DJGPP__)
#  include <map>
#  include <dos.h>
#else
#  include <langinfo.h>
#  define fallback(call) (call)
#endif
#include "charconv.h"

/*

  character conversion (user locale -> latin1) (portable)

  copyright (c) 2005, 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

*/

namespace charset {
    using namespace std;

    namespace {
        union wide {                        // accomodate wstring and string
            wide(wchar_t wc) : code(wc) { }
            wchar_t code;
            char    raw[sizeof(wchar_t)];
        };

        template<class T> inline
        std::basic_string<T>& operator+=(std::basic_string<T>& str, const wide w)
        {
            return str += w.code;
        }

        inline std::string& operator+=(std::string& str, const wide w)
        {
            return str.append(w.raw, sizeof w.raw);
        }
    }

  // latin1 <-> unicode interconversion

    template<> conv<>::data conv<latin1>::decode(const char* s, size_t len)
    {
        conv<>::data build;
        build.reserve(len);
        for( ; len--; ) {
            build += wide(*s++ & 0xFF);
        }
        return build;
    }

    template<> std::string conv<latin1>::encode(const void* p, size_t len)
    {
        const wchar_t* w = (wchar_t*)p;
        std::string build;
        build.reserve(len);
        for( ; len--; ) {
            wchar_t c = *w++;
            build += (c < 0x100)? c : '?';
        }
        return build;
    }

#if !defined(__DJGPP__)

  // locale <-> unicode interconversion
  // a bit touchy when changing locales

    namespace {
        struct _7bit;

        inline bool ok_locale(const char* loc)
        {
            if(loc && ok_locale(0))
                return 1;
#  if defined(__WIN32__)
            char cp[12];
            sprintf(cp, ".%d", GetACP() & 0xFFF);
            loc = cp;
#  endif
            const char* tmp = setlocale(LC_CTYPE, loc);
            return tmp && strcmp(tmp, "C") != 0;
        }

        static bool wchar_unicode()
        {
            static bool const set = ok_locale("");
#  if fallback(1)
#    ifdef CODESET
            return strcmp(nl_langinfo(CODESET), "UTF-8") == 0;
#    else
            return false;
#    endif
#  else
            return true;
#  endif
        }
    } // end anon. namespace

#if fallback(1)

    // fallback conversion, 7bit ASCII <-> unicode
    // (probably should replace this with something based on iconv, some day)

    template<> conv<>::data conv<_7bit>::decode(const char* s, size_t len)
    {
        conv<>::data build;
        build.reserve(len);
        for( ; len--; ) {
            build += wide(*s++ & 0x7F);
        }
        return build;
    }

    template<> std::string conv<_7bit>::encode(const void* p, size_t len)
    {
        const wchar_t* w = (wchar_t*)p;
        std::string build;
        build.reserve(len);
        for( ; len--; ) {
            wchar_t c = *w++;
            build += (c < 0x80)? c : '?';
        }
        return build;
    }

#endif // end-of-ASCII convertor

    template<> conv<>::data conv<local>::decode(const char* s, size_t len)
    {
        if(!wchar_unicode())
            return fallback(conv<_7bit>::decode(s, len));

        conv<>::data build;
        build.reserve(len);
        wchar_t wc;
        s += len;
        for(int n; len; len -= n+!n) {
            n = mbtowc(&wc, s-len, len);
            if(n < 0) break;
            build += wide(wc);
        }
        return build;
    }

    template<> std::string conv<local>::encode(const void* p, size_t len)
    {
        if(!wchar_unicode())
            return fallback(conv<_7bit>::encode(p, len));

        const wchar_t* w = (wchar_t*)p;
        std::string build;
        build.reserve(len*2);

        for( ; len--; ) {
            char buf[MB_LEN_MAX];
            int n = wctomb(buf, *w++);
            if(n >= 0) build.append(buf, n);
            else       build += '?';
        }
        return build;
    }

#elif defined(__DJGPP__)

  // mess-dos codepages (hardcoded, one-on-one relationship to unicode)

    namespace {
        typedef wchar_t charmap[128];

 // Codepage 437, possible alternatives: (those active marked with +)
 //
 // I'd rather have a lunate epsilon or element of for 'î', but it's not WGL
 //
 //   LATIN SMALL L. SHAPR S (00DF) -> GREEK SMALL L. BETA (03B2)
 // + GREEK SMALL L. EPSILON (03B5) -> EURO SIGN  (20AC)
 //   GREEK SMALL L. EPSILON (03B5) -> ELEMENT OF (2208)
 //   GREEK SMALL L. EPSILON (03B5) -> GREEK LUNATE EPSILON S. (03F5)
 //   GREEK SMALL L. PHI     (03C6) -> LATIN SMALL L. O SLASH (00F8)
 //   GREEK SMALL L. PHI     (03C6) -> LATIN SMALL L. PHI (0278)
 //   GREEK SMALL L. PHI     (03C6) -> GREEK PHI S.       (03D5)
 // + BULLET OPERATOR        (2219) -> BULLET (2022)

        const charmap cp437 = {
            0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
            0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
            0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
            0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
            0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
            0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
            0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
            0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
            0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
            0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
            0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
            0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
            0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
            0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x20AC, 0x2229,
            0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
            0x00B0, 0x2022, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
        };

        const charmap cp850 = {
            0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
            0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
            0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
            0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x00D7, 0x0192,
            0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
            0x00BF, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
            0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
            0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
            0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
            0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
            0x00F0, 0x00D0, 0x00CA, 0x00CB, 0x00C8, 0x0131, 0x00CD, 0x00CE,
            0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x00CC, 0x2580,
            0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0x00FE,
            0x00DE, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x00AF, 0x00B4,
            0x00AD, 0x00B1, 0x2017, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
            0x00B0, 0x00A8, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0,
        };

        unsigned codepage()
        {
            REGS cpu;
            cpu.w.ax = 0x6601;    // int21h - ax 6601h - get global code page table
            intdos(&cpu, &cpu);   // -> bx: active code page, dx: system codepage
            return cpu.w.cflag? 0 : cpu.w.bx;  // carry set on error
        }

        const wchar_t* dos_to_uni()
        {
            switch(codepage()) {
            case 850: return cp850;
            case 437: return cp437;
            default : return 0;
            }
        }

        struct uni_to_dos : map<wchar_t, char> {     // crude! reverse table
            uni_to_dos()
            {
                if(const wchar_t* cmap = dos_to_uni())
                    for(int n = 0; n < 128; ++n) {
                        insert(value_type(cmap[n], n|0x80));
                    }
            }
            char& operator[](wchar_t uc)
            {
                return insert(value_type(uc, '?')).first->second;
            }
        };
    }

    template<> conv<>::data conv<local>::decode(const char* s, size_t len)
    {
        static const wchar_t* const map = dos_to_uni();
        conv<>::data build;
        build.reserve(len);
        for( ; len--; s++) {
            wide w = (*s & 0x80)? map[*s & 0x7F] : (*s & 0xFF);
            build += w;
        }
        return build;
    }

    template<> std::string conv<local>::encode(const void* p, size_t len)
    {
        const wchar_t* w = (wchar_t*)p;
        static uni_to_dos rmap;
        std::string build;
        build.reserve(len);
        for( ; len--; ) {
            wchar_t c = *w++;
            build += (c < 0x80)? c : rmap[c];
        }
        return build;
    }

#endif

} // end of namespace

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
   MinGW does the former, Borland C++ the latter. Forcing this with ".ACP" or
   ".OCP", doesn't work on Borland (apparently).

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

  */

