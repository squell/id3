#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

#include "id3v2.hpp"

using namespace std;

inline bool is_id(ID3FRAME f, string id)
{
    return( f->ID == id );
}

int main(int argc, char* argv[])
{
    ID3tag tag(argv[1]);

    ID3tag::iterator i
        = find_if(tag.begin(), tag.end(), bind2nd(ptr_fun(is_id), "APIC"));

    if(i != tag.end()) {
        const char* data = i->data;

        string mime(++data);         // skip encoding byte
        data += mime.length() + 1;   // go past mime type string
        string desc(++data);         // skip pictype byte
        data += desc.length() + 1;   // description string

        cout << desc << " [" + mime + "]" << endl;

        fstream(2[argv], fstream::out | fstream::binary)     // !
        . write( data, i->size - (data-i->data) );
    } else {
        cout << "No APIC frame present" << endl;
    }
}

/*
 ID3v2 APIC frame header:

 Text encoding      $xx
 MIME type          <text string> $00
 Picture type       $xx
 Description        <text string according to encoding> $00 (00)
 Picture data       <binary data>
*/

