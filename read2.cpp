#include <iostream>
#include <string>
#include "id3v2.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    ID3tag tag(argv[1]);

    for(ID3tag::iterator p = tag.begin(); p != tag.end(); p++)
        cout << p->ID << endl << string(p->data,p->size) << endl;
}

