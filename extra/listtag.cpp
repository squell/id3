#include "getid3.h"
#include <iostream>

using namespace std;
using tag::read::ID3;

int main(int argc, char **argv)
{
    ID3 tag(argv[1]);
    ID3::array list = tag.listing();
    for(int i = 0; i < list.size(); i++) {
        string field = list[i].first;
        string data  = list[i].second;
        cout << field << "\t" << data << endl;
    }
}

