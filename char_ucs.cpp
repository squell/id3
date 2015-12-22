#include <string>
#include <climits>
#include "char_ucs.h"

/*

  copyright (c) 2006, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

namespace charset {
    using namespace std;

    namespace {
        union wide {
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

    std::wstring conv_wide::decode(const char* s, size_t len, byte_order ord)
    {
        if(!(len &= ~1U)) return std::wstring();   // force len to 2k, k > 0
        const char* end = s+len;

        std::wstring build;
        build.reserve(len / sizeof(wchar_t));
        bool i = (ord == big_endian);

        switch(wide( s[0^i] & 0xFF | s[1^i]<<8 & 0xFF00U ).code) {
        default: break;
        case 0xFFFE: i = !i;
        case 0xFEFF: s += 2;
        }

        for( ; s < end; s+=2) {
            wide ch( s[0^i] & 0xFF | s[1^i]<<8 & 0xFF00U );
            if(ch.code < 0xD800 || ch.code >= 0xE000)
                build += ch;
            else if(ch.code < 0xDC00 && (s+=2) < end) { // UTF-16 surrogate
                wide lo( s[0^i] & 0xFF | s[1^i]<<8 & 0xFF00U );
                if(lo.code >= 0xDC00 && lo.code < 0xE000)
                    build += wide((ch.code&0x3FF)<<10 | (lo.code&0x3FF) | 0x10000);
            }
        }
        return build;
    }

    string conv_wide::encode(const void* p, size_t len, byte_order ord)
    {
        const wchar_t* w = (wchar_t*)p;
        std::string build;
        build.reserve(len);
        int i = (ord == big_endian) * 8;

        if(ord == marked) {                    // write BOM
            (build += '\xFF') += '\xFE';
        }

        for( ; len--; ) {
            wchar_t c = *w++;
            if(c < 0x10000)                    // innocent warning by gcc
                (build += c>>i & 0xFF) += c>>(8^i) & 0xFF;
            else {                             // encode a UTF16 surrogate pair
                c -= 0x10000;
                wchar_t hi = (c>>10)&0x3FF | 0xD800, lo = c&0x3FF | 0xDC00;
                (build += hi>>i & 0xFF) += hi>>(8^i) & 0xFF;
                (build += lo>>i & 0xFF) += lo>>(8^i) & 0xFF;
            }
        }
        return build;
    }

}

