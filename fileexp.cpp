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

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

namespace fileexp {

struct filefind : record {
    char  mask[PATH_MAX];
    class direxp;

    bool  nested (auto_dir, char*, char*);
    char* pathcpy(char*, const char*);          // special strcpy variant
    find* invoker;
};

bool find::operator()(const char* filemask)
{
    filefind ff;
    strncpy(ff.mask, filemask, sizeof ff.mask); // copy constant
    ff.path[0] = '\0';                          // duct tape
    ff.invoker = this;
    return ff.nested(auto_dir("./"), ff.path, ff.mask);
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

bool filefind::nested(auto_dir dir, char* wpath, char* fnmatch)
{
    typedef vector<string> strvec;
    strvec::size_type prevlen = var.size();     // backup value
    char* nwpath;                               // next write position

    bool w = false;                             // idle check

    while( char* fndirsep = strchr(fnmatch, '/') ) {
        *fndirsep++ = '\0';                     // isolate name part

        nwpath = pathcpy( pathcpy(wpath, fnmatch), "/" );
        if(auto_dir newdir = auto_dir(path)) {
            dir     = newdir;                   // if allready a valid
            wpath   = nwpath;                   //   directory, use as is
            fnmatch = fndirsep;
            continue;                           // (tail recursion)
        }

        while( dirent* fn = dir.read() ) {      // search cur open dir
            direxp match(fnmatch, fn->d_name);
            if(match) {
                nwpath = pathcpy( pathcpy(wpath, fn->d_name), "/" );
                if(auto_dir newdir = auto_dir(path)) {
                    for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                        var.push_back(*i);
                    w = nested(newdir, nwpath, fndirsep) || w;
                    var.resize(prevlen);
                }
            }
        }
        fndirsep[-1] = '/';                     // 'repair' this->mask
        return w;
    }

    bool recursive = invoker->dir(path);

    if(!recursive && access(fnmatch, F_OK) == 0) {
        nwpath = pathcpy(wpath, fnmatch);       // check if file is 'simple'
        invoker->file(fnmatch, *this);          // (speeds up simple cases)
        return true;
    }

    strvec files;
    while( dirent* fn = dir.read() )            // read all files in dir
        files.push_back(fn->d_name);

    sort(files.begin(), files.end());

    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        nwpath = pathcpy(wpath, fn->c_str());
        direxp match(fnmatch, wpath);
        if(match) {
            for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                var.push_back(*i);
            invoker->file(wpath, *this);
            w = true;
            var.resize(prevlen);
        }
        if(recursive && *wpath != '.') {
            if(auto_dir newdir = auto_dir(path))
                w = nested(newdir, pathcpy(nwpath, "/"), fnmatch) || w;
        }
    }

    return w;
}

} // namespace

