#include <stdexcept>
#include <algorithm>
#include "varexp.h"
#include "auto_dir.h"
#if defined(_WIN32)
#    include <io.h>
#    define F_OK 0
#    define X_OK 0
#else
#    include <unistd.h>
#endif
#include "fileexp.h"

/*

  copyright (c) 2004, 2005, 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

#if defined(__DJGPP__) || defined(_WIN32)
#    define readlink(path, buf, n) (-1)         // readlink dummy
#endif

using namespace std;

namespace fileexp {

 /* filefind::nested variables split in two categories: stuff that drives the
    the recursion (parameters); and supporting information (class members) */

struct filefind : record {
    char  mask[PATH_MAX];
    find* invoker;
    char* rec_base;                             // start of recursive search

    class direxp;
    bool  nested (auto_dir, char* pathpos, char* filespec);
    char* pathcpy(char*, const char*);          // special strcpy variant
};

bool find::glob(const char* filemask, bool wildslash)
{
    filefind t;
    strncpy(t.mask, filemask, sizeof t.mask);
    t.path[0]  = t.mask[sizeof t.mask-1] = '\0'; // duct tape
    t.invoker  = this;
    t.rec_base = wildslash? t.path : 0;
#if defined(_WIN32)
    if(strncmp(t.mask, "//", 2) == 0) {         // unc path
        char* shmask = strchr(t.mask+2, '/');
        if(shmask && (shmask=strchr(++shmask, '/'))) {
            *shmask++ = '\0';                   // separate sharename
            char* shpath = t.pathcpy(t.pathcpy(t.path, t.mask), "/");
            if(t.rec_base) t.rec_base = shpath;
            return t.nested(auto_dir(t.mask), shpath, shmask);
        }
        return false;
    } else
#endif
    return t.nested(auto_dir("./"), t.path, t.mask);
}

struct filefind::direxp : varexp {              // special dotfile handling
    inline static bool is_special(const char* fn)
    {
         return (fn[0] == '.') && (!fn[1] || fn[1] == '.' && !fn[2]);
    }

    direxp(const char* mask, const char* test, bool force = false)
    {
#if defined(__DJGPP__) || defined(_WIN32)
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

struct slash_split {
    slash_split(char* p) : ptr(p) { *ptr = '\0'; }
   ~slash_split()                 { *ptr = '/'; }
private:
    char* ptr;
};

  // recursive file search routine, leaves this->mask nonsensical on success
  // pathpos  - write position (inside path)
  // filespec - read position  (inside mask)

bool filefind::nested(auto_dir dir, char* pathpos, char* filespec)
{
    typedef vector<string> strvec;
    strvec::size_type prevlen = var.size();     // backup value
    char* wpos;                                 // next write position

    bool w = false;                             // idle check

    if(!rec_base || rec_base == pathpos)
    while( char* fndirsep = strchr(filespec, '/') ) {
        slash_split lock(fndirsep++);
        wpos = pathcpy(pathcpy(pathpos, filespec), "/");
        if(access(path, X_OK) == 0) {
            dir      = auto_dir(path);          // if allready a valid
            pathpos  = wpos;                    //   directory, use as is
            filespec = fndirsep;                // (tail recursion)
            if(rec_base) rec_base = pathpos;
        } else if(!strpbrk(filespec, "*?[") || !dir) {
            return false;                       // shortcut mismatchers
        } else if(rec_base && strchr(filespec,'*')) {
            break;
        } else {
            while( dirent* fn = dir.read() ) {  // search cur open dir
                direxp match(filespec, fn->d_name);
                if(match) {
                    wpos = pathcpy(pathcpy(pathpos, fn->d_name), "/");
                    if(rec_base) rec_base = wpos;
                    if(auto_dir newdir = auto_dir(path)) {
                        for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                            var.push_back(*i);
                        w |= nested(newdir, wpos, fndirsep);
                        var.resize(prevlen);
                    }
                }
            }
            return w;
        }
    }

    if(! invoker->dir(*this) )
        return false;

    if(*filespec == '\0')
        return invoker->file(pathpos, *this);

    pathcpy(pathpos, filespec);                 // check if file is 'simple'
    if(!strpbrk(filespec, "*?["))
        return access(path, F_OK) == 0 && invoker->file(pathpos, *this);

    if(! dir )                                  // we might not have read access
        return false;

    strvec files;
    while( dirent* fn = dir.read() ) {          // read all files in dir
        if(!direxp::is_special(fn->d_name))
            files.push_back(fn->d_name);
    }

    sort(files.begin(), files.end());            // speeds things up!? wow

    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        wpos = pathcpy(pathpos, fn->c_str());
        direxp match(filespec, rec_base? rec_base : pathpos, !!rec_base);
        if(match) {
            for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                var.push_back(*i);
            w = true, (void)invoker->file(pathpos, *this);
            var.resize(prevlen);
        }
    }

    if(rec_base)
    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        wpos = pathcpy(pathpos, fn->c_str());
        auto_dir newdir(path);
        char sympath[1];                         // dont recurse into symlinks
        if(newdir && readlink(path, sympath, sizeof sympath) < 0)
            w |= nested(newdir, pathcpy(wpos, "/"), filespec);
    }

    pathcpy(pathpos, filespec);                 // try exact match a last resort
    return w || access(path, F_OK) == 0 && invoker->file(pathpos, *this);
}

} // namespace
