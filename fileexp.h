/*

  basic varexp based findfile class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  When a fileexp::find-derived object is used as a function object, it looks
  for files matching the wildcard spec, and calls the overridden members as
  appropriate: file() for each file, dir() for each dir entered.

  dir() should return true or false, indicating whether to perform a recursive
  search in the directory entered.

  The first argument to file() points in the associated, to the start of the
  actual filename sans directory prefix.

  Example:

  struct showfiles : fileexp::find {
      void file(const char*, const fileexp::record& rec)
      { puts(rec.path); }
  };

  int main(int argc, char* argv[])
  {
      showfiles ls;
      for(int x=1; x<argc; ++x)
          ls(argv[x]) || puts("ffind: none found!");
  }

*/

#ifndef __ZF_FILEEXP
#define __ZF_FILEEXP

#include <climits>
#include <vector>
#include <string>

 // multi-directory wildcard search class

namespace fileexp {

    struct record {
        std::vector<std::string> var;         // contains matched vars
        char path[PATH_MAX];                  // contains full file path
    };

    struct find {
        bool operator()(const char* filemask);

        virtual void file(const char* name, const record&) = 0;
        virtual bool dir (const char* path)
        { return 0; }
    };

    struct find_recursive : find {
        virtual bool dir (const char*)
        { return 1; }
    };   

}

#endif

