/*

  smartID3 applicative class - extension for ID3v2

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  See setid3.h for details. This new patch object can patch ID3v2 tags

  New methods in this class:

  .set(ID, raw content)
      Set an ID3v2 specific tag directly (overrides ID3v1)
      Does not chain back, e.g. setting "TIT2" does not set ID3v1 "title".
  .rm(ID)
      Remove specific ID3v2 frames from a tag.
  .v1 .v2
      Wether or not to write a ID3v1 / ID3v2 tag.
      Default: write ID3v2 only.

  Limitation/Flaws:

  smartID3v2 is unaware of all the specific semantics of the ID3v2 frames,
  for the sake of simplicity. This does not pose a real problem as the
  responsibility of filling an ID3v2 frame with correct content now lies
  with the user of this class, however, it does have as a consequence that
  .vmodify() doesn't know about frames having multiple instances.

  a. - when updating a frame type which has multiple instances, the first one
       will be replaced and the rest will be left unaltered.
    
  b. - smartID3v2 will never add multiple instances of the same frame type.
  
  c. - deleting a frame using .rm() deletes *ALL* frames of a specific type.

  (a) requires an extra distinguisher function.
  (b) is not a real problem, but it could be solved by porting to multimap
  (c) is what users would expect, therefore intended behaviour

*/

#ifndef __ZF_SETID3V2
#define __ZF_SETID3V2

#include <string>
#include <map>

#include "setid3.h"

class smartID3v2 : public smartID3 {
public:
    smartID3v2(bool w1 = 0, bool w2 = 1) : v1(w1), v2(w2)
    { }

    smartID3v2& set(ID3set i, const char* m);

    smartID3v2& set(std::string i, std::string m)     // set ID3v2 frame
    { mod2[i] = m; return *this; }

    smartID3v2& rm(std::string i)                     // remove ID3v2 frame
    { mod2[i].erase(); return *this; }

    smartID3v2& opt(bool n1, bool n2)                 // toggle v1/v2 writer
    { v1 = n1, v2 = n2; return *this; }

    bool v1, v2;

protected:
    virtual bool vmodify(const char*, const base_container&) const;

private:
    typedef std::map<std::string,std::string> db;
    db mod2;
};

#endif

