#include "printid3.h"

 /* lists tags using the 'printid3' interface */

int main(int argc, char **argv)
{
    id3p_listhead();
    while(argc--)
        id3p_showfile(*++argv);
    id3p_listfoot();
}

