#include <string>
#include <vector>
#include <cstdio>
#include "getid3v2.h"
#include "charconv.h"
#include "id3v2.h"

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

using set_tag::read::ID3v2;
using set_tag::ID3field;

ID3v2::ID3v2(const char* fn) : tag(ID3_readf(fn,0))
{
}

ID3v2::~ID3v2()
{
    ID3_free(tag);
}

static bool getframe(ID3FRAME f, void* tag, const char* field)
{
    for( ; *field != '\0'; field+=4) {
        ID3_start(f, tag);
        while(ID3_frame(f))
            if(strncmp(field, f->ID, 4) == 0) {
                if(f->ID[0] == 'T') {
                    if(f->size > 1) return 1;
                } else
                    return 1;
            }
    }
    return 0;
}

 // sigh.

static cvtstring unbinarize(ID3FRAME f)
{
    const char sep[] = ": ";
    if(f->ID[0] == 'T' && f->ID[3] != 'X' ||
       strcmp(f->ID, "IPLS") == 0 || strcmp(f->ID, "USER") == 0) {
        int skip((f->ID[0]=='U')? 4 : 1);
        string s(f->data+skip, f->size-skip);
	switch(f->data[0]) {
	case 0:  return cvtstring::latin1(s);
	default: return cvtstring::latin1("?");
	}
    } else if(f->ID[0] == 'W' && f->ID[3] != 'X') {
        string s(f->data+1, f->size-1);
        return cvtstring::latin1(s);
    } else if(strcmp(f->ID, "COMM") == 0 || strcmp(f->ID, "USLT") == 0 ||
              strcmp(f->ID, "TXXX") == 0 || strcmp(f->ID, "WXXX") == 0) {
        int skip((f->ID[3]=='X')? 1 : 4);
        string s(f->data+skip, f->size-skip);
        int    i;
        switch(f->data[0]) {
        case 0 :            //    s = cvtstring::latin1(s).latin(); no-op
            break;
        default:
            return cvtstring::latin1("?");
        }
        s.erase(i = s.find('\0'), 1);
        if(i > 0) s.insert(i, sep);
        return cvtstring::latin1(s);
    } else if(strcmp(f->ID, "PCNT") == 0) {
	unsigned long t = 0;
	char buf[12];			 // long enough to hold a 32bit digit
	t = (f->data[0] & 0xFF) << 24 | (f->data[1] & 0xFF) << 16 |
	    (f->data[2] & 0xFF) <<  8 | (f->data[3] & 0xFF);
        sprintf(buf, "%lu", t);
	return cvtstring::latin1(buf);
    }
    return cvtstring::latin1("<unsupported>");
}

 // double sigh.

cvtstring ID3v2::operator[](ID3field field) const
{
    ID3FRAME f;
    bool ok = false;
    if(tag)
	switch( field ) {
        case title:
            ok = getframe(f, tag, "TIT2" "TIT3" "TOFN" "TRSN");
	    break;
	case artist:
            ok = getframe(f, tag, "TPE1" "TPE2" "TPE3" "TPE4" "TCOM"
                                  "TEXT" "TOPE" "TOLY" "TPUB" "TRSO");
	    break;
        case album:
            ok = getframe(f, tag, "TALB" "TPOS" "TOAL");
	    break;
	case year:
            ok = getframe(f, tag, "TYER" "TORY" "TRDA" "TDAT");
	    break;
	case cmnt:
            ok = getframe(f, tag, "COMM" "WPUB" "TCOP" "WCOP");
	    break;
	case track:
            ok = getframe(f, tag, "TRCK" "TIME");
	    break;
	case genre:
            ok = getframe(f, tag, "TCON" "TIT1");
        }
    return ok? unbinarize(f) : cvtstring();
}

/* ====================================================== */

ID3v2::array ID3v2::listing() const
{
    ID3FRAME f;
    ID3_start(f, tag);

    array vec;
    vec.push_back( array::value_type("ID3v2", "2.3") );
    while(ID3_frame(f)) {
	vec.push_back( array::value_type(f->ID, unbinarize(f)) );
    }
    return vec;
}

