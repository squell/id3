#include <cstdio>
#include <cctype>
#include <ctime>                                    // borland needs this
#include <sys/stat.h>
#include <utime.h>
#if defined(__WIN32__)
#    include <io.h>
#    define F_OK 0
#else
#    include <unistd.h>
#endif
#include "sedit.h"
#include "setfname.h"

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

    utimbuf* get_utime(const char* fname, utimbuf* buf)
    {
        struct stat file_info;
        if( stat(fname, &file_info) == 0 ) {
            buf->actime  = file_info.st_atime;
            buf->modtime = file_info.st_mtime;
            return buf;
        }
        return 0;
    }
}

bool file::vmodify(const char* fname, const function& edit) const
{
    utimbuf buf, *stamp = get_utime(fname, &buf);

    function::result edited = edit(m_template);    // use pre-values

    if(m_template.empty() || !edited.good()) {
        bool ok = group::vmodify(fname, edit);
        if(m_preserve) utime(fname, stamp);
        return ok;
    }

    string name = edited;
    for(string::iterator p = name.begin(); p != name.end(); ++p) {
        if(!portable_fn(*p)) *p = '_';             // replace ill. chars
    }
    if(const char* psep = strrchr(fname, '/')) {
        name.insert(0, fname, psep-fname+1);       // copy path prefix
    }

    const char* newfn = name.c_str();
    if(access(newfn, F_OK) == 0)                          // check if exists
        throw failure("file already exists ", newfn);

    bool ok = group::vmodify(fname, edit);
    if(m_preserve) utime(fname, stamp);

    if(ok && std::rename(fname, newfn) != 0)
        throw failure("could not rename ", fname);

    return ok;
}

}

