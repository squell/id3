#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <string>
#include <map>
#include <algorithm>
#include "setid3.h"
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
    { 'T', 'A', 'G' }
};

 /* this is a packing check. if the check works out fine, any decent compiler
    should be able to optimize this fully away. */

const char _pack_error[] = "ID3v1 tag structure not packed! Recompile!";

static struct _pack_check {
    _pack_check()
    {
        if(sizeof(ID3v1) != 128) {
            puts(_pack_error);
            exit(1/!(sizeof(ID3v1)-128));  // generate compiler warning too
        }
    }
} _do;

/* ====================================================== */

// Capitalize A Text-String Like This.

static string capitalize(string s)
{
    bool new_w = true;
    for(string::iterator p = s.begin(); p != s.end(); p++) {
        new_w = !isalpha( *p = (new_w? toupper:tolower)(*p) ); // heh =)
    }
    return s;
}

/* ====================================================== */

const struct genre_map : map<string,int> {
    typedef const_iterator iter;                // shorthand

    genre_map()                                 // initialize associative map
    {
        for(int i=0; ID3v1_genres[i]; i++) {
            (*this)[ capitalize(ID3v1_genres[i]) ] = i;
        }
    }
} ID3_genre;

/* ====================================================== */

string smartID3::edit(string s, const base_container& v)
{
    int pos = 0;

    while( (pos=s.find(VAR, pos)) >= 0 ) {
        bool und = false;
        bool cap = false;
        char hex = 0;
        int n = 1;
        while( pos+n < s.length() ) {
            switch( char c = toupper(s[pos+n]) ) {
            default:
                s.erase(pos, n);
                break;
            case ':':
                s.erase(pos, 1);
                s[pos++] = '\0';
                break;
            case VAR:       // "%%" -> "%"
                s.erase(pos, 1);
                ++pos;
                break;
            case '_':
                und = true;
                ++n;
                continue;
            case 'C':
                cap = true;
                ++n;
                continue;
            case '0':
                c += 10;      // so it'll turn up as 9 when we subtract '1'
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                string tmp = v[c-'1'];
                if(cap)
                    tmp = capitalize(tmp);
                if(!und)
                    replace(tmp.begin(), tmp.end(), '_', ' ');
                s.replace(pos, n+1, tmp);
                pos += tmp.length();
                break;
            }
            break;
        }
    }
    return s;
}

/* ====================================================== */

bool smartID3::vmodify(const char* fn, const base_container& v)
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
            genre_map::iter g = ID3_genre.find( capitalize(edit(txt,v)) );
            tag.genre = (g==ID3_genre.end() ? 255 : g->second);
        }

        if( fresh && count(mod.begin(),mod.end(),(char*)0) == 7 ) {
            ftrunc(f);
        } else {
            fwrite(&tag, 1, 128, f);
        }

        fclose(f);
        return 1;
    };

    return 0;
}

/*

 annotated bug:

 Under MS-DOS with DJGPP, if an fwrite() crossed the end of the file, and no
 fseek() has occured, the buffer space(??) of the last fread() will be
 appended to the file before the data actually written by fwrite(). (?)

 ARGH!

*/

