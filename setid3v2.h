/*

  smartID3 applicative class - extension for ID3v2

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  See setid3.h

*/

#ifndef __ZF_SETID3V2
#define __ZF_SETID3V2

#include <string>
#include <map>

#include "setid3.h"

class smartID3v2 : public smartID3 {
public:
    smartID3v2()
    { }

    smartID3v2& set(std::string i, std::string m)     // set ID3v2 frame
    { mod2[i] = m; return *this; }

    smartID3v2& rm(std::string i)                     // remove ID3v2 frame
    { mod2[i] = ""; }

    smartID3v2& set(ID3set i, const char* m);

protected:
    virtual bool vmodify(const char*, const base_container&);

private:
    typedef std::map<std::string,std::string> db;
    db mod2;
};

#endif

