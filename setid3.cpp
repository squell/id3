#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <string>
#include <map>
#include <algorithm>
#include <new>
#include "setid3.h"
#include "sedit.h"
#include "id3v1.h"

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#if defined(__WIN32__)
#    include <io.h>
#    define ftrunc(f)  chsize(fileno(f), ftell(f))
#else
#    include <unistd.h>
#    define ftrunc(f)  ftruncate(fileno(f), ftell(f))
#endif

using namespace std;

const ID3v1 synth_tag = {
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

bool clipped_compare(const string& is, const string& js)
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

const struct genre_map : map<string,int,bool (*)(const string&,const string&)> {
    typedef const_iterator iter;                // shorthand

    genre_map()                                 // initialize associative map
    : map<string,int,key_compare>( clipped_compare )
    {
        for(int i=0; i < ID3v1_numgenres; i++) {
            if(i==80)  (*this)[ "Folk0" ] = i;  // small kludges ;)
            if(i==100) (*this)[ "Humo" ] = i;
            else       (*this)[ capitalize(ID3v1_genre[i]) ] = i;
        }
    }
} ID3_genre;

/* ====================================================== */

bool smartID3::vmodify(const char* fn, const base_container& v) const
{
    ID3v1 tag = { { 0 } };                    // duct tape

    if( FILE* f = fopen(fn, "rb+") ) {
        fseek(f, -128, SEEK_END);
        fread(&tag, 128, 1, f);
        fseek(f,    0, SEEK_CUR);             // * BUG * annotated below

        if( memcmp(tag.TAG, "TAG", 3) == 0 )
            fseek(f, -128, SEEK_END);         // overwrite existing tag
        else
            tag = synth_tag;                  // create new tag

        if(fresh) tag = synth_tag;

        const char* txt;                      // reading aid

        if(txt = mod[title])
            strncpy(tag.title,  edit(txt,v).c_str(), sizeof tag.title);

        if(txt = mod[artist])
            strncpy(tag.artist, edit(txt,v).c_str(), sizeof tag.artist);

        if(txt = mod[album])
            strncpy(tag.album,  edit(txt,v).c_str(), sizeof tag.album);

        if(txt = mod[year])
            strncpy(tag.year,   edit(txt,v).c_str(), sizeof tag.year);

        if(txt = mod[cmnt])
            strncpy(tag.cmnt,   edit(txt,v).c_str(), sizeof tag.cmnt);

        if(txt = mod[track])
            tag.track = atoi( edit(txt,v).c_str() );

        if(txt = mod[genre]) {
            unsigned int    x = atoi(txt) - 1;
            genre_map::iter g = ID3_genre.find( capitalize(edit(txt,v)) );
            tag.genre = (g==ID3_genre.end() ? x : g->second);
        }

        bool err;

        if( fresh && count(mod.begin(),mod.end(),(char*)0) == 7 ) {
            err = ftrunc(f) != 0;
        } else {
            err = fwrite(&tag, 1, 128, f) != 128;
        }

        fclose(f);

        if(err) {
            string emsg("error writing ID3 tag to ");
            throw failure(emsg + fn);
        }

        return 1;
    };

    return 0;
}

/* ====================================================== */

smartID3::failure::failure(const string& s)
: txt( new (nothrow) char[s.length()+1] )
{
    if(txt.get())
        strcpy(txt.get(), s.c_str());
}

/*

 annotated bug:

 Under MS-DOS with DJGPP, if an fwrite() crossed the end of the file, and no
 fseek() has occured, the buffer space(??) of the last fread() will be
 appended to the file before the data actually written by fwrite(). (?)

 ARGH!

*/

