/*

  tag::read::ID3v2

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The read::ID3v2 class implements the reader interface for ID3v2 tags

*/

#ifndef __ZF_GETID3V2
#define __ZF_GETID3V2

#include <string>
#include <cctype>
#include "set_base.h"

namespace tag {
    namespace read {

        class ID3v2 : public metadata {
        public:
            const void* const tag;

            typedef metadata::factory<ID3v2> factory;
            explicit ID3v2(const char* fn);
            explicit ID3v2(const void* id3v2_data);
           ~ID3v2();
            value_string operator[](ID3field field) const;
            array listing() const;
            operator bool() const { return tag; }

            static bool has_lang(const std::string field)  // implies has_enc
            { return field == "COMM" || field == "COM" ||
                     field == "USLT" || field == "ULT" ||
                     field == "USER"; }

            static bool has_desc(const std::string field)  // implies has_enc
            { return field == "TXXX" || field == "TXX" ||
                     field == "WXXX" || field == "WXX" ||
                     field == "COMM" || field == "COM" ||
                     field == "USLT" || field == "ULT" ||
                     field == "POPM" || field == "POP"; }

            static bool is_counter(const std::string field)
            { return field == "PCNT" || field == "CNT" ||
                     field == "POPM" || field == "POP"; }

            static bool is_url(const std::string field)
            { return field[0] == 'W'; }

            static bool is_text(const std::string field)
            { return field[0] == 'T' || field == "IPLS" || field == "IPL"; }

            static bool is_valid(const std::string field)
            {
                 using namespace std;
                 string::size_type n;
                 for(n = 0; n < field.length(); ++n) {
                     if(!isupper(field[n]) && !isdigit(field[n]))
                         return false;
                 }
                 return n == 3 || n == 4;
            }
        };

    }
}

#endif

