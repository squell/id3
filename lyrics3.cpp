#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <new>
#include "lyrics3.h"
#include "id3v1.h"

/*

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

#if defined(_WIN32)
#    include <io.h>
#    define ftrunc(f)  chsize(fileno(f), ftell(f))
#else
#    include <unistd.h>
#    define ftrunc(f)  ftruncate(fileno(f), ftell(f))
#endif

using namespace std;

namespace lyrics3 {

/* ====================================================== */

  // increases position in a string to where the next field would be

info::size_type find_next(const info& s, info::size_type pos)
{
    if(pos+8 < s.size()) {
        const string& tmp = s.substr(pos+3, 5);
        char* p;
        unsigned long n = strtoul(tmp.c_str(), &p, 10);
        if(*p != '\0') return false;
        return (pos + 8 + n);
    }
    return false;
}

  // searches a tag. given that most lyrics3 tags will consist of just a
  // few frames, a raw linear search is efficient enough

string find(const info& s, const string& sig)
{
    string::size_type i, next;

    for(i = 0; next=find_next(s, i); i = next)
        if(s.substr(i, 3) == sig)
            return i+=8, s.substr(i, next - i);

    return string();
}

  // validates a string as valid lyrics3v2 content

info cast(const string& s)
{
    string::size_type i, next;

    for(i = 0; next=find_next(s, i); i = next)
        if(!isupper(s[i]) || !isupper(s[i+1]) || !isupper(s[i+2])) break;

    return i == s.size()? s : string();
}

/* ====================================================== */

inline string num(unsigned long n, int width)
{
    char buf[12];
    if(sprintf(buf, "%0*lu", width, n & 0xFFFFFFFFul) == width)
        return buf;
    return string();
}

info field(const string& id, const string& content)
{
    if(id.length() != 3 || !isupper(id[0]) || !isupper(id[1]) || !isupper(id[2]))
        return info();
    const string& size = num(content.size(),5);
    return size.empty()? size : id + size + content;
}

/* ====================================================== */

  // seeks the start of a lyrics tag

size_t seek_start(FILE* f, char id3[128])
{
    char buf[15];                          // "xxxxxxLYRICS200"

    if( fseek(f, -128, SEEK_END) == 0   &&
        fread(id3, 1, 128, f)    == 128 &&
        memcmp(id3, "TAG", 3)    == 0) {
        if( fseek(f, -15-128, SEEK_END) != 0 ) return 0;
    } else {
        *id3 = '\0';
        clearerr(f);
        if( fseek(f, -15,     SEEK_END) != 0 ) return 0;
    }

    buf[sizeof buf-1] = '\0';              // duct tape
    fread(buf, 1, 15, f);                  // read end-tag

    if(memcmp(buf+6, "LYRICS200", 9) == 0) {
        char* p;
        long size = strtoul(buf, &p, 10);
        if(p == buf+6) {
            if(fseek(f, -15 - size, SEEK_CUR) != 0) return 0;
            fread(buf, 1, 11, f);
            if(memcmp(buf, "LYRICSBEGIN", 11) == 0)
                return size;
        }
    }

    return 0;
}

  // read a lyrics3v2 tag to a std::string

info read(const char* fn, void* id3)
{
    FILE* f = fopen(fn, "rb");
    if( !f ) return string();

    char tmp[128];
    id3 || (id3=tmp);

    if(size_t size = seek_start(f,(char*)id3)) {
        struct scope {
            char* data; ~scope() { delete[] data; }
        } tmp = { new (nothrow) char[size -= 11] };
        if(tmp.data && fread(tmp.data, 1, size, f) == size) {
            fclose(f);
            string data(tmp.data, size);
            return cast(data);
        }
    }
    fclose(f);
    return string();
}

/* ====================================================== */

  // copy extended Lyrics3 tags to a ID3v1 tag

inline void override_id3(ID3v1& id3v1, const info& tag)
{
    const char extended_tag[][4] = { "ETT", "EAR", "EAL" };
    char (*const tag_ptr[])[30] = { &id3v1.title, &id3v1.artist, &id3v1.album };
    for(int i=0; i < 3; ++i) {
        const string field = find(tag, extended_tag[i]);
        if(!field.empty())
            strncpy(*tag_ptr[i], field.c_str(), 30);
    }
}

  // write a lyrics3v2 string to a file

int write(const char* fn, const info& tag, const void* newid3)
{
    FILE* f = fopen(fn, "rb+");
    if( !f ) return 1;

    union {
        ID3v1 id3_info;
        char id3[128];
    };

    int result;

    if( seek_start(f,id3) ) {
        result = fseek(f, -11, SEEK_CUR) == 0;
    } else {
        clearerr(f);
        result = fseek(f, id3[0]?-128:0, SEEK_END) == 0;
    }

    if(newid3) memcpy(id3, newid3, 128);
    override_id3(id3_info, tag);

    if(result++) {
        const string& s = "LYRICSBEGIN"+tag+num(tag.size()+11,6)+"LYRICS200";
        if(s.size() == tag.size()+11+6+9) {
            if(tag.size() > 0)
                fwrite(s.data(), 1,  s.size(), f);
            if(id3[0])
                fwrite(id3,      1,       128, f);
            result = -(ferror(f) || ftrunc(f) != 0);
        }
    }

    return fclose(f) | result;
}

}

