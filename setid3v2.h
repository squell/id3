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
    smartID3v2(bool w1 = 0, bool w2 = 1) : v1_(w1), v2_(w2)
    { }

    smartID3v2& set(std::string i, std::string m)     // set ID3v2 frame
    { mod2[i] = m; return *this; }

    smartID3v2& rm(std::string i)                     // remove ID3v2 frame
    { mod2[i].erase(); return *this; }

    smartID3v2& v1(bool f)                            // toggle ID3v1 writer
    { v1_ = f; return *this; }

    smartID3v2& v2(bool f)                            // toggle ID3v2 writer
    { v2_ = f; return *this; }

    smartID3v2& set(ID3set i, const char* m);

protected:
    virtual bool vmodify(const char*, const base_container&);

private:
    typedef std::map<std::string,std::string> db;

    bool v1_, v2_;
    db mod2;
};

#endif

