#include "id3v2.h"

 /* deletes ID3v2 tags from files */

int main(int argc, char *argv[])
{
    char zero = 0;
    while(argc--)
       ID3_writef(*(++argv), &zero);
}
