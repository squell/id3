#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "varexp.h"
#include "auto_dir.h"
#include "ffindexp.h"

/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

using namespace std;

struct filefindexp::direxp : varexp {           // special dotfile handling
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

char* filefindexp::pathcpy(char* dest, const char* src)
{
    do {                                        // added safety
        if(dest == &path[sizeof path])
            throw length_error("filefindexp: pathname error");
    } while(*dest++ = *src++);
    return dest-1;                              // return *resume*-ptr
}

bool filefindexp::operator()(const char* filemask)
{
    strncpy(mask, filemask, sizeof mask);       // copy constant
    path[0] = '\0';                             // duct tape
    return nested(auto_dir("./"), path, mask);
}

  // recursive file search routine, leaves this->mask nonsensical on success
  // wpath   - write position (inside path)
  // fnmatch - read position  (inside mask)

bool filefindexp::nested(auto_dir dir, char* wpath, char* fnmatch)
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

    entered();

    strvec files;
    while( dirent* fn = dir.read() )            // read all files in dir
        files.push_back(fn->d_name);

    sort(files.begin(), files.end());

    for(strvec::iterator fn = files.begin(); fn != files.end(); ++fn) {
        pathcpy(wpath, fn->c_str());
        direxp match(fnmatch, wpath);
        if(match) {
            for(varexp::iterator i = match.begin(); i != match.end(); ++i)
                var.push_back(*i);
            process();
            w = true;
            var.resize(prevlen);
        }
    }

    return w;
}

