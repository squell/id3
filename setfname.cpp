#include <cstdio>
#include <string>
#include "setfname.h"
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

namespace set_tag {

bool filename::vmodify(const char* fname, const base_container& v) const
{
    if(!ftemplate)
        return false;

    std::string name = edit(ftemplate, v);

    if(const char* psep = strrchr(fname, '/')) {      // copy path prefix
        name.insert(0, fname, psep-fname+1);
    }

    const char* newfn = name.c_str();

    if(access(newfn, F_OK) == 0)                      // check if file exists
        throw failure("file already exists ", newfn);

    if(std::rename(fname, newfn) != 0)
        throw failure("could not rename ", fname);
}

 // that's all folks! :)

}

