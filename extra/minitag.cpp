#include "setid3.h"

 /* mini mini tagger */

int main(int argc, char* argv[])
{
    smartID3()
    .clear()
    .set(artist, "%2")
    .set(title,  "%3")
    .modify(argv[1], argv);
}

