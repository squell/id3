#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <string>
#include <map>
#include <algorithm>
#include "setid3.h"

#if defined(__WIN32__)
#    include <io.h>
#    define ftrunc(f)  chsize(fileno(f), ftell(f))
#else
#    include <unistd.h>
#    define ftrunc(f)  ftruncate(fileno(f), ftell(f))
#endif

/*

  (c) 2000 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

struct ID3v1 {                                   // ID3 v1.1 tag structure
    char TAG[3];
    char title [30];
    char artist[30];
    char album [30];
    char year[4];
    char cmnt[28];
    char __;
    char track;
    char genre;
};

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

#include "genres"                               // ID3v1_genres list

static struct genre_map : map<string,int> {
    genre_map()                                 // initialize associative map
    {
        for(int i=0; i < sizeof ID3v1_genres/sizeof *ID3v1_genres; i++) {
            (*this)[ capitalize(ID3v1_genres[i]) ] = i;
        }
    }
} ID3_genre;

/* ====================================================== */

string smartID3::edit(string s, const base_container& v)
{
    int pos = 0;
    int i;

    while( (pos=s.find('%', pos)) >= 0 && pos+1 < s.length() ) {
        bool c = toupper(s[pos+1]) == 'C';              // caps modifier flag

        if(c && pos+2 >= s.length()) break;             // bounds check

        if(i = s[pos+1+c], i>='0' && i<='9') {          // replace %0 .. %9
            const string& tmp = c ? capitalize(v[i-'0']) : v[i-'0'];
            s.replace(pos, 2+c, tmp);
            pos += tmp.length();
        } else if(s[pos+1] = '%') {                     // "%%" -> "%"
            s.erase(pos,1);
            ++pos;
       }
    }
    replace(s.begin(), s.end(), '_', ' ');              // remove _'s
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

        if(txt = mod[genre])
            tag.genre = ID3_genre[capitalize(edit(txt,v))];

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

