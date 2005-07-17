#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include "varexp.h"
#include "auto_dir.h"
#include "fileexp.h"
#if defined(__WIN32__)
#    include <io.h>
#    define F_OK 0
#else
#    include <unistd.h>
#endif

/*

  (c) 2004, 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#if defined(__DJGPP__) || defined(__WIN32__)
#    define readlink(path, buf, n) (-1)         // readlink dummy
#endif

using namespace std;

namespace fileexp {

struct filefind : record {
    char  mask[PATH_MAX];
    bool  recurse;
    class direxp;

    bool  nested (auto_dir, char* pathpos, char* filespec);
    char* pathcpy(char*, const char*);          // special strcpy variant
    find* invoker;
};

bool find::operator()(const char* filemask)
{
    filefind t;
    strncpy(t.mask, filemask, sizeof t.mask);   // copy constant
    t.path[0] = t.mask[sizeof t.mask-1] = '\0'; // duct tape
    t.invoker = this;
    t.recurse = false;
    return t.nested(auto_dir("./"), t.path, t.mask);
}

struct filefind::direxp : varexp {              // special dotfile handling
    direxp(const char* mask, const char* test)
    {
        if(test[0] != '.')
            result = match(mask, test);
        else if(mask[0] == '.') {
            if(!test[1] || test[1] == '.' && !test[2])
                result = 0;                     // ignore '.' and '..'
            else
                result = match(mask, test);
        } else
            result = 0;
    }
};

char* filefind::pathcpy(char* dest, const char* src)
{
    do {                                        // added safety
        if(dest == &path[sizeof path])
            throw length_error("filefindexp: pathname error");
    } while(*dest++ = *src++);
    return dest-1;                              // return *resume*-ptr
}

  // recursive file search routine, leaves this->mask nonsensical on success
  // wpath   - write position (inside path)
  // fnmatch - read position  (inside mask)

bool filefind::nested(auto_dir dir, char* pathpos, char* filespec)
{
    typedef vector<string> strvec;
    strvec::size_type prevlen = var.size();     // backup value
    char* wpos;                                 // next write position

    bool w = false;                             // idle check

    while( char* fndirsep = strchr(filespec, '/') ) {
        *fndirsep++ = '\0';                     // isolate name part

        wpos = pathcpy( pathcpy(pathpos, filespec), "/" );
        if(auto_dir newdir = auto_dir(path)) {
            dir      = newdir;                  // if allready a valid
            pathpos  = wpos;                    //   directory, use as is
            filespec = fndirsep;
            continue;                           // (tail recursion)
        }

        while( dirent* fn = dir.read() ) {      // search cur open dir
            direxp match(filespec, fn->d_name);
            if(match) {
                wpos = pathcpy( pathcpy(pathpos, fn->d_name), "/" );
                if(auto_dir newdir = auto_dir(path)) {
                    for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                        var.push_back(*i);
                    w = nested(newdir, wpos, fndirsep) || w;
                    var.resize(prevlen);
                }
            }
        }
        fndirsep[-1] = '/';                     // 'repair' this->mask
        return w;
    }

    bool recursive = invoker->dir(path);

    if(!recursive && access(filespec, F_OK) == 0) {
        wpos = pathcpy(pathpos, filespec);      // check if file is 'simple'
        invoker->file(filespec, *this);         // (speeds up simple cases)
        return true;
    }

    strvec files;
    while( dirent* fn = dir.read() )            // read all files in dir
        files.push_back(fn->d_name);

    sort(files.begin(), files.end());

    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        wpos = pathcpy(pathpos, fn->c_str());
        direxp match(filespec, pathpos);
        if(match) {
            for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                var.push_back(*i);
            invoker->file(pathpos, *this);
            w = true;
            var.resize(prevlen);
        }
        char buf[1];
        if(recursive && *pathpos != '.' && readlink(path, buf, 1) < 0) {
            if(auto_dir newdir = auto_dir(path))
                w = nested(newdir, pathcpy(wpos, "/"), filespec) || w;
        }
    }

    return w;
}

} // namespace

