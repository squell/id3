#include <new>
#include <algorithm>
#include <functional>
#include <cctype>
#include <cstdlib>
#include "char_ucs.h"
#include "id3v1.h"
#include "id3v2.h"
#include "fileops.h"
#include "getid3v2.h"
#include "setid3v2.h"

/*

  copyright (c) 2004, 2005, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Note: I'm devoting quite a bit of code to glue the interface from C to C++,
  and I'm not entirely happy about it. I'm a bit confused as to the reasons I
  didn't chose to write id3v2.c in C++. :(

*/

using namespace std;

using tag::write::ID3v2;
using tag::ID3field;

namespace {

    typedef int concreteness_check[ sizeof ID3v2() ];

/* ===================================== */

 // extra hairyness to prevent buffer overflows by re-allocating on the fly
 // overkill, but i had to do a runtime check anyway, so.

    class writer {
        size_t avail;
        char *base, *dest;
        ID3VER version;                     // writes ID3v2.3 per default

    public:
        void init(size_t len, ID3VER v = ID3_v2_3)
                         { base = (char*) malloc(avail=len+!len);
                           if(!base) throw bad_alloc();
                           dest = (char*) ID3_put(base,version=v,0,0,0); }

        operator char*() { return base; }

        writer()         { base = 0; }
       ~writer()         { free(base); }

        void put(const char* ID, const void* src, size_t len);

    private:
        writer(writer&);                    // non-copyable
        void operator=(writer&);
    };

    void writer::put(const char* ID, const void* src, size_t len)
    {
        static size_t factor = 0x1000;      // start reallocing in 4k blocks

        if(len+10 > avail) {
            while(len+10 > factor) factor *= 2;
            size_t size = dest - base;
            base     = (char*) realloc(base, size+factor);
            avail    = factor;
            if(!base) throw bad_alloc();
            dest     = base + size;         // translate current pointer
        }

        avail -= (len+10);
        dest = (char*) ID3_put(dest, version, ID, src, len);
    }

/* ===================================== */

 // convert C handler to a C++ exception at program startup

    extern "C" void copy_failure(const char*, const char*);

    struct guard {
        guard()            { ID3_wfail = copy_failure; }
        static string err;
        static void raise();
    } static fail_inst;

    string guard::err;

    void guard::raise()
    {
        string emsg;
        emsg.swap(err);
        if(!emsg.empty())
            throw tag::failure(emsg);
    }

    extern "C" void copy_failure(const char* oldn, const char* newn)
    {
        if(oldn == newn) {
            string emsg("error writing ID3 to ");
            guard::err = emsg + oldn;
        } else {
            string emsg(": file lost, contents still in `");
            guard::err = newn + emsg + oldn + '\'';
        }
    }

}

/* ===================================== */

 // code for constructing ID3v2 frames. rather hairy, but hey, ID3v2 sucks

inline static bool needs_unicode(charset::conv<wchar_t> str)
{
    const wstring& ws = str;
    return ws.end() != find_if(ws.begin(), ws.end(), bind2nd(greater<wchar_t>(), 0xFF));
}

static const string encode(int enc, const charset::conv<>& str)
{
    using charset::conv;
    switch(enc) {
    case 0:
        return conv<charset::latin1>(str);
    case 1:
        return conv<charset::utf16>(str);
    /*
    case 2:
        return conv<charset::utf16be>(str);
    case 3:
        return conv<charset::utf8>(str);
    */
    }
    throw enc;
}

  // split "FIELD[:descr[:lan]]" into three components
  // returns actual descriptor via result

static string split_field(string& field, string& lang)
{
    string descr;
    const string::size_type colon = field.find(':');       // split description
    if(colon != string::npos) {
        descr = field.substr(colon+1);
        field.erase(colon);
    }
    const string::size_type E = descr.size();              // split language
    if(E >= 4 && descr[E-4] == ':' && isalpha(descr[E-3]) && isalpha(descr[E-2]) && isalpha(descr[E-1])) {
        lang = descr.substr(E-3);
        descr.erase(E-4);
    }
    return descr;
}

 // returns empty string if unsupported

static const string binarize(string field, charset::conv<charset::latin1> content)
{
    using tag::read::ID3v2;
    using charset::conv;

    string lang = "xxx";
    const conv<char>& descr = split_field(field, lang) += '\0';

    if(field == "TCON" || field == "TCO") {                // genre by number
        unsigned int x = atoi(string(content).c_str())-1;  // is portable
        if(x < ID3v1_numgenres) content = ID3v1_genre[x];
    }

    string data;
    if(!ID3v2::is_valid(field))
        return data;
    if(ID3v2::is_counter(field)) {
        string s = content;
        unsigned long t = strtoul(s.c_str(), 0, 0);
        if(ID3v2::has_desc(field)) {
            data.append(encode(0,descr));
            data.push_back(t&0xFFu);
            string::size_type i = s.find(':');
            t = i==string::npos? 0 : strtoul(s.substr(i+1).c_str(), 0, 0);
        }
        data.push_back(t >> 24 & 0xFF);
        data.push_back(t >> 16 & 0xFF);
        data.push_back(t >>  8 & 0xFF);
        data.push_back(t       & 0xFF);
        return data;
    }

    // figure out of latin1 is enough to encode strings
    data = char(needs_unicode(content + descr));

    if(ID3v2::has_lang(field)) {
        data.append(lang);
    }
    if(ID3v2::has_desc(field)) {
        data.append(encode(data[0], descr));
    } else if(descr.length() > 1) {
        return string();
    }

    if(data.length() > 1 || ID3v2::is_text(field)) {
        const string blob = ID3v2::is_url(field)? string(content) : encode(data[0], content);
        return data.append(blob);
    } else if(ID3v2::is_url(field)) {
        return content;
    } else {
        return string();
    }
}

/* ===================================== */

typedef map<string,string> db;

ID3v2& ID3v2::set(ID3field i, string m)
{
    static const char xlat2[][4] = {                     // ID3field order!
        "TT2", "TP1", "TAL", "TYE", "COM", "TRK", "TCO"
    };
    static const char xlat3[][5] = {
        "TIT2", "TPE1", "TALB", "TYER", "COMM", "TRCK", "TCON"
    };
    if(i < FIELD_MAX) {
        set(xlat2[i], m);      // let error handling decide between them.
        set(xlat3[i], m);
    }
    return *this;
}

ID3v2& ID3v2::reserve(size_t n)
{
    resize = n? n : 1;
    return *this;
}

bool ID3v2::from(const char* fn)
{
    ID3_free(null_tag);
    return null_tag = (fn? ID3_readf(fn, 0) : 0);
}

static void make_canonical(string& field)
{
    using tag::read::ID3v2;
    string lang = "xxx";
    string descr = split_field(field, lang);
    if(ID3v2::has_lang(field))
        field += ':' + descr + ':' + lang;
    else if(ID3v2::has_desc(field) || !descr.empty())
        field += ':' + descr;
}

bool ID3v2::set(string field, string s)
{
    make_canonical(field);
    if( binarize(field, "0").length() != 0 ) {      // test a dummy string
        mod[field] = s;
        return true;
    }
    return false;
}

bool ID3v2::rm(string field)
{
    make_canonical(field);
    mod[field].erase();
    return true;
}

/* ===================================== */

tag::metadata* ID3v2::read(const char* fn) const
{
    return new read::ID3v2(fn);
}

namespace tag {
    extern read::ID3v2::value_string unbinarize(ID3FRAME, charset::conv<>& descriptor);
}

static inline bool has_enc(const char* ID)
{
    return tag::read::ID3v2::is_text(ID) || tag::read::ID3v2::has_desc(ID);
}

bool ID3v2::vmodify(const char* fn, const function& edit) const
{
    size_t check;
    read::ID3v2 info( ID3_readf(fn, &check) );

    if(!info.tag) {
        if(check != 0)
            return false;                           // evil ID3 tag
        if(!force)
            return true;
    }

    const void* src = fresh? null_tag : info.tag;
    ::writer tag;
    db table(mod);

    if( src ) {                                     // update existing tags
        ID3FRAME f;
        tag.init(0x1000, ID3_start(f, src));

        while(ID3_frame(f)) {
            string field = f->ID;
            if(read::ID3v2::has_desc(f->ID)) {
                charset::conv<char> descr;
                tag::unbinarize(f, descr);
                field += descr;
            }
            db::iterator p = table.find(field);
            if(p == table.end()) {
                if(has_enc(f->ID) && *f->data > 1U) {
                    charset::conv<char> _;          // recode v2.4 text to UTF16
                    const string b = binarize(field, tag::unbinarize(f, _));
                    tag.put(f->ID, b.data(), b.length());
                } else
                    tag.put(f->ID, f->data, f->size);
            } else {
                if(!p->second.empty()) {            // else: erase frames
                    if(function::result s = edit(p->second)) {
                        const string b = binarize(p->first, s);
                        tag.put(f->ID, b.data(), b.length());
                    } else {
                        tag.put(f->ID, f->data, f->size);
                    }
                    table.erase(p);
                }
            }
        }
    } else {
        tag.init(0x1000);
    }

    for(db::iterator p = table.begin(); p != table.end(); ++p) {
        if(!p->second.empty()) {
            if(function::result s = edit(p->second)) {
                const string field = p->first.substr(0, p->first.find(':'));
                const string b = binarize(p->first, s);
                tag.put(field.c_str(), b.data(), b.length());
            }
        }
    }

    bool result = ID3_writef(fn, tag, resize);
    guard::raise();                                 // deferred exception?

    return result;
}

ID3v2::~ID3v2()
{
    if(null_tag) ID3_free(null_tag);
}

