/*

  basic varexp based findfile class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  When a filefindexp-derived object is used as a function object, it looks for
  files matching the wildcard spec, and calls the overridden process() member
  for each match. The member variables 'path' and 'var' contain the pathname
  and matched variables (see varexp), and should not be modified.

  entered() is called whenever a directory is entered to be scanned for files

  Example:

  struct showfiles : filefindexp {
      void process()
      { puts(path); }
  };

  int main(int argc, char* argv[])
  {
      showfiles ls;
      for(int x=1; x<argc; ++x)
          ls(argv[x]) || puts("ffind: none found!");
  }

*/

#ifndef __ZF_FFINDEXP
#define __ZF_FFINDEXP

#include <climits>
#include <vector>
#include <string>
#include "auto_dir.h"

 // multi-directory wildcard search class

class filefindexp {
public:
    bool operator()(const char* filemask);
protected:
    virtual void process() = 0;               // override this one
    virtual void entered() { }                // def. do nothing

    std::vector<std::string> var;             // contains matched vars
    char path[PATH_MAX];                      // contains full file path

private:
    bool nested(auto_dir, char*, char*);
    char* pathcpy(char*, const char*);        // special strcpy variant

    char mask[PATH_MAX];
    class direxp;
};

#endif

