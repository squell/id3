#include "setid3v2.h"

int main(int argc, char* argv[])
{
    smartID3v2()
//  .set("TPE1", "%2")
//  .set("TIT2",  "%3")
//  .set("TIT2", "Anguish")
    .clear()
    .modify(argv[1], argv);
}

