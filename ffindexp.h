/*

  varexp based findfile class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  When a ffindexp object is used as a function object, it looks for files
  matching the wildcard spec, and calls the function it is initialized with
  for each match. The member variables 'path' and 'var' contain the
  pathname and matched variables (see varexp).

  Example:

  void show(const filefindexp& f)
  {
      puts(f.path);
  }

  filefindexp(show) showfiles;

  int main(int argc, char* argv[])
  {
      if(! showfiles(argv[1]) ) {
          puts("None found!");
      }
  }

*/

#ifndef __ZF_FFINDEXP
#define __ZF_FFINDEXP

#if defined(__WIN32__)
#  define _POSIX_ 1                // borland c++ needs this
#endif
#include <climits>

#include <vector>
#include <string>
#include "auto_dir.h"

 // multi-directory wildcard search class

class filefindexp {
public:
    typedef void (*function_type)(const filefindexp&);

    filefindexp(function_type f) : process(f) { }
    bool operator()(const char* filemask);

    std::vector<std::string> var;             // contains matched vars
    char path[PATH_MAX];                      // contains full file path
private:
    bool nested(auto_dir dir, char* wpath, char* fnmatch);
    char* pathcpy(char*, const char*);        // special strcpy variant

    const function_type process;
    char mask[PATH_MAX];
    class direxp;
};

#endif

