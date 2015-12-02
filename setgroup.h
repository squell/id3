#ifndef __ZF_SETGROUP_HPP
#define __ZF_SETGROUP_HPP

#include <vector>
#include "set_base.h"

namespace tag {

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // run-time delegation using standard container      //
  ///////////////////////////////////////////////////////

#pragma clang diagnostic ignored "-Wempty-body"

template<class Interface>
class combined : public Interface, private std::vector<Interface*> {
public:
  // publish some methods
    typedef Interface                                        interface;
    typedef typename std::vector<interface*>::size_type      size_type;
    typedef typename std::vector<interface*>::iterator       iterator;
    typedef typename std::vector<interface*>::const_iterator const_iterator;
#ifndef __BORLANDC__
    using std::vector<interface*>::begin;
    using std::vector<interface*>::end;
    using std::vector<interface*>::size;
    using std::vector<interface*>::operator[];
#else
    const_iterator begin() const { return std::vector<interface*>::begin(); }
    iterator       begin()       { return std::vector<interface*>::begin(); }
    const_iterator end  () const { return std::vector<interface*>::end(); }
    iterator       end  ()       { return std::vector<interface*>::end(); }
    size_type      size () const { return std::vector<interface*>::size(); }
    interface*     operator[](size_type index) const
    { return std::vector<interface*>::operator[](index); }
    interface*&    operator[](size_type index)
    { return std::vector<interface*>::operator[](index); }
#endif

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
        bool result = size() == 0;
        for(const_iterator p = begin(); p != end(); )
            result |= (*p++)->from(fn);
        return result;
    }

protected:
    bool vmodify(const char* fn, const stredit::function& func) const
    {
        bool success = true;
        for(const_iterator p = begin(); p != end(); )
            success &= (*p++)->modify(fn, func);
        return success;
    }
};

}

#endif

