#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <map>
#include <new>
#include "id3v1.h"
#include "getid3.h"
#include "setid3.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

*/

#if defined(__WIN32__)
#    include <io.h>
#    define ftrunc(f)  chsize(fileno(f), ftell(f))
#else
#    include <unistd.h>
#    define ftrunc(f)  ftruncate(fileno(f), ftell(f))
#endif

using namespace std;

using tag::write::ID3;
using tag::ID3field;

static ID3v1 const zero_tag = {
    { 'T', 'A', 'G' },
    "",  // title
    "",  // artist
    "",  // album
    "",  // year
    "",  // cmnt
    0,
    0,   // track
    255  // genre
};

/* ====================================================== */

static string str_upper(string s)
{
    for(string::iterator p = s.begin(); p != s.end(); ++p)
        *p = toupper(*p);
    return s;
}

/* ====================================================== */

 /*
    This is basically an enhanced lexicographical_compare which ignores
    certain parts of a string during comparison:

    Any plain character compared against a seperator gets 'eaten'
    e.g. "Alt Rock" will match "Alternative Rock" exactly.

    Every seperator matches every other seperator.
    e.g. "Fast Fusion" matches "Fast-Fusion"
  */

static inline bool issep(int c)
{
    return !isalnum(c);
}

static bool clipped_compare(const string& is, const string& js)
{
    string::const_iterator i = is.begin();
    string::const_iterator j = js.begin();

    for( ; i != is.end() && j != js.end(); ++i, ++j) {
        if(issep(*i)) {
            if(!issep(*j)) --i;
        } else {
            if( issep(*j)) --j; else {
                if(*i < *j)
                   return true;
                if(*i > *j)
                   return false;
            }
        }
    }
    return find_if(i, is.end(), issep) == is.end() &&
           find_if(j, js.end(), issep) != js.end();
}

/* ====================================================== */

struct genre_map : map<string,int,bool (*)(const string&,const string&)> {
    typedef const_iterator iter;                // shorthand

    genre_map()                                 // initialize associative map
    : map<string,int,key_compare>( clipped_compare )
    {
        (*this)[ "Psych" ] = 67;                // small kludges
        (*this)[ "Folk0" ] = 80;
        (*this)[ "Humo"  ] = 100;
        for(int i=0; i < ID3v1_numgenres; i++) {
            (*this)[ str_upper(ID3v1_genre[i]) ] = i;
        }
    }
} const ID3_genre;

/* ====================================================== */

bool ID3::from(const char* fn)
{
    delete (ID3v1*)null_tag, null_tag = 0;
    if(fn) {
        read::ID3 src(fn);
        if(src) null_tag = new ID3v1(src.tag);
    }
    return null_tag;
}

 // note: Intel C++ produces an internal error here if the base class has
 // the return type as 'const reader*' and this class doesn't. apparently
 // caused by some compiler bug in relation to multiple inheritance.

tag::metadata* ID3::read(const char* fn) const
{
    return new read::ID3(fn);
}

 // note: Borland doesn't suppress array-to-pointer conversion when deducing
 // reference type parameters (in this case), so use pointer instead

template<size_t N>
static inline bool setfield(char (*dest)[N], const charset::conv<>* src)
{
    if(src)
        return strncpy(*dest, src->template c_str<charset::latin1>(), N);
    else
        return false;
}

bool ID3::vmodify(const char* fn, const function& edit) const
{
    const ID3v1& synth_tag = null_tag? *(ID3v1*)null_tag : zero_tag;

    ID3v1 tag = { { 0 } };                    // duct tape

    if( FILE* f = fopen(fn, "rb+") ) {
        fseek(f, -128, SEEK_END);
        fread(&tag, 1, 128, f);               //  annotated below
        fseek(f,    0, ftell(f)<128? SEEK_END : SEEK_CUR);

        if( ferror(f) ) {
            fclose(f);
            return false;
        }

        if( memcmp(tag.TAG, "TAG", 3) == 0 )
            fseek(f, -128, SEEK_END);         // overwrite existing tag
        else
            tag = synth_tag;                  // create new tag

        if( cleared ) tag = synth_tag;

        using namespace charset;
        const string* field;                  // reading aid
        int n = bool(null_tag);               // count number of set fields

        if(field = update[title])
            n += setfield(&tag.title,  edit(*field));

        if(field = update[artist])
            n += setfield(&tag.artist, edit(*field));

        if(field = update[album])
            n += setfield(&tag.album,  edit(*field));

        if(field = update[year])
            n += setfield(&tag.year,   edit(*field));

        if(field = update[cmnt]) {
            n += setfield(&tag.cmnt,   edit(*field));
            if(tag.zero != '\0')
                tag.track = tag.zero = 0;               // ID3 v1.0 -> v1.1
        }
        if(field = update[track]) {
            if(function::result rs = edit(*field)) {
                ++n, tag.track = atoi( rs.c_str<latin1>() );
                tag.zero = '\0';
            }
        }
        if(field = update[genre]) {
            if(function::result rs = edit(*field)) {
                string s          = str_upper( rs.str<latin1>() );
                unsigned int x    = atoi(s.c_str()) - 1;
                genre_map::iter g = ID3_genre.find(s);
                tag.genre = (s.empty() || g==ID3_genre.end()? x : g->second);
                ++n;
            }
        }

        bool err;

        if( cleared && n == 0 ) {
            err = ftrunc(f) != 0;
        } else {
            err = fwrite(&tag, 1, 128, f) != 128;
        }

        fclose(f);

        if(err)
            throw failure("error writing TAG to ", fn);

        return true;
    };

    return false;
}

ID3::~ID3()
{
    delete (ID3v1*)null_tag;
}

/*

 annotated bug:

 Under MS-DOS with DJGPP, if an fwrite() crossed the end of the file, and no
 fseek() has occured, the buffer space(??) of the last fread() will be
 appended to the file before the data actually written by fwrite(). (?)

 ARGH!

 Actually, this is C conformant behaviour - no write shall follow a read
 without an intervening fflush or fseek.

*/

