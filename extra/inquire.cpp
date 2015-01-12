#include <cstdio>
#include <cstring>
#include <exception>
#include "mass_tag.h"
#include "setecho.h"
#include "getid3.h"
#include "getid3v2.h"

using namespace std;
using namespace fileexp;
using namespace set_tag;

int main(int argc, const char* argv[])
{
    if(argc != 3) {
        printf("usage: <format string> <file-spec>\n");
        return 1;
    }

    try {
        return mass_tag(echo(argv[1]), read::ID3::factory()).glob(argv[2]);
    } catch(const exception& exc) {
        printf("exception: %s\n", exc.what());
    }
}

