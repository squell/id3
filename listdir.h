/*

  Simple dirent.h wrapper

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage example:

    listdir d(path);
    if(!d)
       printf("invalid directory");
    else
       while(char* fn = d.read()) puts(fn);

  Copy-construction invalidates the source of the copy (Like auto_ptr)

*/

#ifndef __ZF_LISTDIR_H
#define __ZF_LISTDIR_H

#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>              // for some implementations of dirent.h
#endif
#include <dirent.h>

class listdir {
public:
    inline listdir(const char* path);         // open a directory
    inline operator bool();                   // instance is valid?
    inline ~listdir();

    inline void rewind();
    inline char* read();

    inline listdir(listdir&);                 // copy construction
private:
    DIR* dirp;

    inline void operator=(listdir&) { }       // no assignment
};

listdir::listdir(const char* path)
{
    dirp = opendir(path);
}

listdir::~listdir()
{
    if(dirp) closedir(dirp);
}

listdir::operator bool()
{
    return dirp;
}

void listdir::rewind()
{
    rewinddir(dirp);
}

char* listdir::read()
{
    if( dirent* fn = readdir(dirp) )
        return fn->d_name;
    else
        return 0;
}

listdir::listdir(listdir& y)
{
    dirp = y.dirp;
    y.dirp = 0;
}

#endif
