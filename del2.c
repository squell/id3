#include <stdio.h>
#include "id3v2.h"

int main(int argc, char *argv[])
{
    char zero = 0;
    ID3_writef(argv[1], &zero);
}

