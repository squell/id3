/*

  auto_dir smart pointer class

  copyright (c) 2004 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  Encapsulates POSIX dirent.h functionality in a basic pointer class
  very much like auto_ptr<>

*/

#ifndef __ZF_AUTO_DIR
#define __ZF_AUTO_DIR

#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>              // for some implementations of dirent.h
#endif
#include <dirent.h>

 // simple c++ wrapper for handling dirent.h

class auto_dir {
    struct ref { auto_dir* p; };            // passing to/from functions
    DIR* dirp;
public:
    explicit auto_dir(const char* path)  { dirp = opendir(path); }
   ~auto_dir()                           { if(dirp) closedir(dirp); }

    operator bool() const                { return dirp; }
    dirent* read()                       { return readdir(dirp); }
    void rewind()                        { rewinddir(dirp); }

    auto_dir(auto_dir& other)            { dirp = other.release(); }
    auto_dir& operator=(auto_dir& other) { reset(other.release()); return *this; }

    operator ref()                       { ref tmp = { this }; return tmp; }
    auto_dir(ref r)                      { dirp = r.p->release(); }
    auto_dir& operator=(ref r)           { return (*this = *r.p); }

    DIR* release()
    { DIR* tmp(dirp); dirp = 0; return tmp; }
    void reset(DIR* p = 0)
    { if(dirp!=p) closedir(dirp); dirp = p; }
};

#endif

