/*

  set_tag::ID3v2 applicative class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The set_tag::ID3v2 class implements the single_tag interface for ID3 tags

  Limitation/Flaws:

  ID3v2 is unaware of all the specific semantics of the ID3v2 frames, for
  the sake of simplicity. This does not pose a real problem as the
  responsibility of filling an ID3v2 frame with correct content now lies
  with the user of this class, however, it does have as a consequence that
  .vmodify() doesn't know about frames having multiple instances.

  a. - when updating a frame type which has multiple instances, the first one
       will be replaced and the rest will be left unaltered.

  b. - ID3v2 will never add multiple instances of the same frame type.

  c. - deleting a frame using .rm() deletes *ALL* frames of a specific type.

  (a) requires an extra distinguisher function.
  (b) is not a real problem, but it could be solved by porting to multimap
  (c) is what users would expect, therefore intended behaviour

*/

#ifndef __ZF_SETID3V2
#define __ZF_SETID3V2

#include <string>
#include <map>

#include "set_base.h"

namespace set_tag {

class ID3v2 : public single_tag {
    std::map<std::string,std::string> mod;
    size_t resize;

    bool check_field(std::string&, std::string&);
public:
    ID3v2(bool f = true) : single_tag(f), mod(), resize(0) { }

    ID3v2& set(ID3field i, const char* m);        // set standard field
    ID3v2& reserve(size_t);                       // set suggested size

    virtual bool vmodify(const char*, const base_container&) const;

  // extended set

    bool set(std::string field, std::string s); // set ID3v2 frame
    bool rm(std::string field);                 // remove ID3v2 frame
};

}

#endif

