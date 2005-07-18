/*

  varexp based findfile class

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  find::glob() and find::pattern() look for files matching the wildcard spec,
  and call the overridden members as appropriate: file() for each file, dir()
  for each dir entered. After a call to dir(), all successive calls to file()
  are for files to that dir.

  glob() searches for files matching the wildcard spec like a POSIX shell
  would, pattern() searches a directory hierarchy recursively for pathnames
  matching the wildcard spec.

  dir() should return true or false, indicating whether to process the
  directory passed as an argument.

  The first argument to file() points in the associated record, to the start
  of the actual filename sans directory prefix.

  Example:

  struct showfiles : fileexp::find {
      void file(const char*, const fileexp::record& rec)
      { puts(rec.path); }
  };

  int main(int argc, char* argv[])
  {
      showfiles ls;
      for(int x=1; x<argc; ++x)
          ls.glob(argv[x]) || puts("ffind: none found!");
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
        bool glob   (const char* filemask);
        bool pattern(const char* root, const char* pathmask);

        virtual void file(const char* name, const record&) = 0;
        virtual bool dir (const char* path)
        { return 1; }

        virtual ~find() { }
    };

}

#endif

