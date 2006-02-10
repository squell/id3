/*

  varexp based findfile class

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  find::glob() and find::pattern() look for files matching the wildcard spec,
  and call the overridden members as appropriate: file() for each file, dir()
  for each dir files are searched in. After a call to dir(), all successive
  calls to file() are for files to that dir.

  glob() searches for files matching the wildcard spec like a POSIX shell
  would, pattern() searches a directory hierarchy recursively for pathnames
  matching the wildcard spec. During a single glob() or pattern(), the record
  argument will always refer to the same object.

  dir() should return true or false, indicating whether to process the
  directory passed as an argument. file() should return true or false
  depending on the result of the operation.

  The first argument to file() aliasses with the associated record::path,
  pointing to the start of the actual filename sans directory prefix.

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

        virtual bool file(const char* name, const record&) = 0;
        virtual bool dir (const record&)
        { return 1; }

        virtual ~find() { }
    };

}

#endif

