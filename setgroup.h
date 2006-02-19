#ifndef __ZF_SETGROUP_HPP
#define __ZF_SETGROUP_HPP

#include <string>
#include <vector>
#include "set_base.h"

namespace tag {

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // (delegates all messages to registered handlers)   //
  ///////////////////////////////////////////////////////

class combined : public handler, private std::vector<handler*> {
    mutable handler::body  data;
    mutable body::nullable basefn;
public:
  // registers a delegate tag
    combined& add(handler& h)
    { push_back(&h); return *this; }
    combined& forget(size_type pos, size_type num = 1)
    { erase(begin()+pos, begin()+pos+num); return *this; }

  // standard state set methods
    combined& clear(bool t = true)
    { data.clear(t);  return *this; }
    combined& set(ID3field i, std::string m)
    { data.set(i, m); return *this; }

    bool vmodify(const char*, const stredit::function&) const;
    bool from(const char*);

  // publish some methods
    using std::vector<handler*>::iterator;
    using std::vector<handler*>::begin;
    using std::vector<handler*>::end;
    using std::vector<handler*>::size;
    using std::vector<handler*>::operator[];
};

}

#endif
