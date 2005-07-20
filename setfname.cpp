#include <cstdio>
#include <cctype>
#include "setfname.h"
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

using namespace std;

namespace set_tag {

    // checks if a character is a member of the portable set

namespace {
    const char allowed[] = " ._-()";
    bool portable_fn(char c)
    {
        return isalnum(c) || (c & 0x80) || strchr(allowed, c);
    }
}

bool filename::vmodify(const char* fname, const subst& v) const
{
    if(ftemplate.empty())
        return false;

    string name = edit(ftemplate, v, "Unknown").local();

    for(string::iterator p = name.begin(); p != name.end(); ++p) {
        if(!portable_fn(*p)) *p = '_';
    }

    if(const char* psep = strrchr(fname, '/')) {      // copy path prefix
        name.insert(0, fname, psep-fname+1);
    }

    const char* newfn = name.c_str();

    if(access(newfn, F_OK) == 0)                      // check if file exists
        throw failure("file already exists ", newfn);

    if(std::rename(fname, newfn) != 0)
        throw failure("could not rename ", fname);

    return true;
}

}

