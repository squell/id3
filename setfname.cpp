#include <cstdio>
#include <cstring>
#include <cctype>
#include <ctime>                                    // borland needs this
#include <sys/stat.h>
#if defined(_MSC_VER)
#    include <sys/utime.h>
#else
#    include <utime.h>
#endif
#if defined(_WIN32)
#    include <io.h>
#    define F_OK 0
#else
#    include <unistd.h>
#endif
#include "sedit.h"
#include "setfname.h"

/*

  copyright (c) 2004-2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

*/

using namespace std;

using tag::write::file;

    // checks if a character is a member of the portable set

namespace {
    // \/?*":|<> are special char (fat)
    const char allowed[] = " !#$%&'()+,-.;=@[]^_`{}~";
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
        bool ok = combined<interface>::vmodify(fname, edit);
        if(m_preserve) utime(fname, stamp);
        return ok;
    }

    string name = edited;
    int dot = 1;
    for(string::iterator p = name.end()-1; p != name.begin()-1; --p) {
        if(!portable_fn(*p)) *p = '_';             // replace ill. chars
        if(*p == '.') {
            if(dot-- <= 0) {
                *p = '_'; 
            }
        }
    }
    if(const char* psep = strrchr(fname, '/')) {
        name.insert(0, fname, psep-fname+1);       // copy path prefix
    }

    const char* newfn = name.c_str();
    if(name != fname && access(newfn, F_OK) == 0)  // check if already exists
        throw failure(newfn, ": file already exists");

    bool ok = combined<interface>::vmodify(fname, edit);
    if(m_preserve) utime(fname, stamp);

    if(ok && std::rename(fname, newfn) != 0)
        throw failure("could not rename ", fname);

    return ok;
}

