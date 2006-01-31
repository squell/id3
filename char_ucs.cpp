#include <string>
#include <climits>
#include "char_ucs.h"

/*

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

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

    conv<>::data conv_ucs::decode(const char* s, size_t len, byte_order ord)
    {
        if(!(len &= ~1)) return wstring();        // degenerate
        const char* end = s+len;

        wstring build;
        build.reserve(len / sizeof(wchar_t));
        bool i = (ord == big_endian);

        switch(wide( s[0^i] & 0xFF | s[1^i]<<8 & 0xFF00U ).code) {
        default: break;
        case 0xFFFE: i = !i;
        case 0xFEFF: s += 2;
        }

        for( ; s < end; s+=2) {
            build += wide( s[0^i] & 0xFF | s[1^i]<<8 & 0xFF00U );
	}
	return build;
    }

    string conv_ucs::encode(const void* p, size_t len, byte_order ord)
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
            if(c < 0x10000)
                (build += c>>i & 0xFF) += c>>(8^i) & 0xFF;
            else
                (build += '?'>>i) += '?'>>(8^i);
        }
	return build;
    }

}

