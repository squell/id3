#ifndef __ZF_SETGROUP_HPP
#define __ZF_SETGROUP_HPP

#include <vector>
#include "set_base.h"

namespace tag {

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // run-time delegation using standard container      //
  ///////////////////////////////////////////////////////

template<class Interface>
class combined : public Interface, private std::vector<Interface*> {
public:
  // publish some methods
    typedef Interface                                        interface;
    typedef typename std::vector<interface*>::size_type      size_type;
    typedef typename std::vector<interface*>::iterator       iterator;
    typedef typename std::vector<interface*>::const_iterator const_iterator;
    using std::vector<interface*>::begin;
    using std::vector<interface*>::end;
    using std::vector<interface*>::size;
    using std::vector<interface*>::operator[];

  // registers tags
    combined& with(interface& object)
    { this->push_back(&object); return *this; }
    combined& ignore(size_type pos, size_type num = 1)
    { this->erase(begin()+pos, begin()+pos+num); return *this; }

  // handler functions
    combined& set(ID3field f, std::string s)
    { for(iterator p = begin(); p != end(); (*p++)->set(f, s));  return *this; }
    combined& rewrite(bool b = true)
    { for(iterator p = begin(); p != end(); (*p++)->rewrite(b)); return *this; }
    combined& create(bool b = true)
    { for(iterator p = begin(); p != end(); (*p++)->create(b));  return *this; }

    bool from(const char* fn)
    {
        bool result = false;
        for(const_iterator p = begin(); p != end(); )
            result |= (*p++)->from(fn);
        return result;
    }

protected:
    bool vmodify(const char* fn, const stredit::function& func) const
    {
        bool result = false;
        for(const_iterator p = begin(); p != end(); )
            result |= (*p++)->modify(fn, func);
        return result;
    }
};

}

#endif

