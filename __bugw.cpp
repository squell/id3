#include <stdio.h>

int main()
{
    char tag[256];
    char sql[] = "squell";

    FILE* f = fopen("test2.mp3", "rb+");
    fread(&tag, 128, 1, f);
    printf("%d\n", ftell(f));
//  fseek(f, 0, SEEK_CUR);         // overwrite existing tag
    fwrite(&sql, 1, 6, f);
    printf("%d\n", ftell(f));
}

