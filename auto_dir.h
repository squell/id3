/*

  auto_dir smart pointer class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

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
    explicit auto_dir(const char* path) throw()
    { dirp = opendir(path); }
   ~auto_dir() throw()
    { if(dirp) closedir(dirp); }

    operator bool() throw()
    { return dirp; }
    dirent* read() throw()
    { return readdir(dirp); }
    void rewind() throw()
    { rewinddir(dirp); }

    DIR* release() throw()
    { DIR* tmp(dirp); dirp = 0; return tmp; }
    void reset(DIR* p = 0) throw()
    { if(dirp!=p) closedir(dirp); dirp = p; }

    auto_dir(auto_dir& other) throw()
    { dirp = other.release(); }
    auto_dir& operator=(auto_dir& other) throw()
    { reset(other.release()); return *this; }

    operator ref() throw()
    { ref tmp = { this }; return tmp; }
    auto_dir(ref r) throw()
    { dirp = r.p->release(); }
    auto_dir& operator=(ref r) throw()
    { return (*this = *r.p); }
};

#endif

