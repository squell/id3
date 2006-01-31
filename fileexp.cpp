#include <stdexcept>
#include <algorithm>
#include "varexp.h"
#include "auto_dir.h"
#if defined(__WIN32__)
#    include <io.h>
#    define F_OK 0
#else
#    include <unistd.h>
#endif
#include "fileexp.h"

/*

  copyright (c) 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  see the accompanying file 'COPYING' for license conditions

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

bool find::glob(const char* filemask)
{
    filefind t;
    strncpy(t.mask, filemask, sizeof t.mask);
    t.path[0] = t.mask[sizeof t.mask-1] = '\0'; // duct tape
    t.invoker = this;
    t.recurse = false;
    return t.nested(auto_dir("./"), t.path, t.mask);
}

bool find::pattern(const char* root, const char* pathmask)
{
    filefind t;
    char* p = t.pathcpy(t.path, root);
    if(p == t.path || p[-1] != '/') {           // append slash?
        p = t.pathcpy(p, "/");
    }
#ifdef AUTO_PREFIX_PATH
    strncpy(t.mask,            t.path,   sizeof t.mask);
    strncpy(t.mask+(p-t.path), pathmask, sizeof t.mask-(p-t.path));
#else
    strncpy(t.mask, pathmask, sizeof t.mask);   // behave as GNU find does
#endif
    t.mask[sizeof t.mask-1] = '\0';             // duct tape
    t.invoker = this;
    t.recurse = true;
    auto_dir start(t.path);
    return start && t.nested(start, p, t.mask);
}

struct filefind::direxp : varexp {              // special dotfile handling
    inline static bool is_special(const char* fn)
    {
         return (fn[0] == '.') && (!fn[1] || fn[1] == '.' && !fn[2]);
    }

    direxp(const char* mask, const char* test, bool force = false)
    {
#if defined(__DJGPP__) || defined(__WIN32__)
        if(force || !is_special(test))
            result = match(mask, test);
        else
            result = 0;
#else                                           // conform to Bourne shell
        if(force || test[0] != '.' || mask[0] == '.')
            result = match(mask, test);
        else
            result = 0;
#endif
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
  // pathpos  - write position (inside path)
  // filespec - read position  (inside mask)

bool filefind::nested(auto_dir dir, char* pathpos, char* filespec)
{
    typedef vector<string> strvec;
    strvec::size_type prevlen = var.size();     // backup value
    char* wpos;                                 // next write position

    bool w = false;                             // idle check

    if(!recurse) while( char* fndirsep = strchr(filespec, '/') ) {
        *fndirsep++ = '\0';                     // isolate name part

        wpos = pathcpy(pathcpy(pathpos, filespec), "/");
        if(auto_dir newdir = auto_dir(path)) {
            dir      = newdir;                  // if allready a valid
            pathpos  = wpos;                    //   directory, use as is
            filespec = fndirsep;
            continue;                           // (tail recursion)
        }

        while( dirent* fn = dir.read() ) {      // search cur open dir
            direxp match(filespec, fn->d_name);
            if(match) {
                wpos = pathcpy(pathcpy(pathpos, fn->d_name), "/");
                if(auto_dir newdir = auto_dir(path)) {
                    for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                        var.push_back(*i);
                    w |= nested(newdir, wpos, fndirsep);
                    var.resize(prevlen);
                }
            }
        }
        fndirsep[-1] = '/';                     // 'repair' this->mask
        return w;
    }

    if(! invoker->dir(*this) )
        return false;

    if(*filespec == '\0')
        return invoker->file(pathpos, *this);

    if(!recurse && access(filespec, F_OK) == 0) {
        pathcpy(pathpos, filespec);             // check if file is 'simple'
        return invoker->file(pathpos, *this);   // (speeds up simple cases)
    }

    strvec files;
    while( dirent* fn = dir.read() ) {          // read all files in dir
        if(!direxp::is_special(fn->d_name))
            files.push_back(fn->d_name);
    }

    sort(files.begin(), files.end());            // speeds things up!? wow

    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        wpos = pathcpy(pathpos, fn->c_str());
        direxp match(filespec, recurse? path : pathpos, recurse);
        if(match) {
            for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                var.push_back(*i);
            w |= invoker->file(pathpos, *this);
            var.resize(prevlen);
        }
    }

    if(recurse) for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        wpos = pathcpy(pathpos, fn->c_str());
        auto_dir newdir(path);
        char sympath[1];
        if(newdir && readlink(path, sympath, sizeof sympath) < 0)
            w |= nested(newdir, pathcpy(wpos, "/"), filespec);
    }

    return w;
}

} // namespace

