#ifndef __ZF_SETGROUP_HPP
#define __ZF_SETGROUP_HPP

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <algorithm>
#include "set_base.h"

namespace tag {

template<class T0 = void, class T1 = void, class T2 = void, class T3 = void,
         class T4 = void, class T5 = void, class T6 = void, class T7 = void>
struct group;

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // compile time delegation - template metaprograms   //
  ///////////////////////////////////////////////////////

struct nihil { };

  // encapsulate a type, and control the virtual signatures it introduces

template<class T, class Interface = nihil>
struct use : Interface {
    typedef T type, dynamic;
    dynamic object;

    explicit use(type init = type()) : object(init) { }
    template<class Init>
      explicit use(Init val) : object(val) { }
                                  
    template<class P> bool modify          // non-virtual pass-throughs
      (const char* fn, const P& var) const { return object.modify(fn, var); }
    bool modify (const char* fn) const     { return object.modify(fn); }
    use& set    (ID3field f, std::string s){ return object.set(f,s),   *this; }
    use& rewrite(bool t = true)            { return object.rewrite(t), *this; }
    use& create (bool t = true)            { return object.create(t),  *this; }
    bool from   (const char* fn)           { return object.from(fn),   *this; }
    metadata* read(const char* fn) const   { return object.read(fn); }
protected:
    bool vmodify(const char* fn, const stredit::function& f) const
    { return object.modify(fn, f);     }
};

  // transforms a recursively nested type into a boxed-type hierarchy
  // - on a decent compiler, this will only emit code if ::dynamic is used
  // - passing only "messages" through this would be better, but creating
  // - those messages is involved as well

template<class L, class R>
struct use< std::pair<L,R> > : use<L>, use<R> {
    typedef std::pair<L, R> type;
    typedef use<use, handler> dynamic;

    template<class U> explicit use(U& init) : use<L>(init), use<R>(init) { }
    use() { }

    bool modify(const char* fn) const
    { return use<L>::modify(fn)      | use<R>::modify(fn);     }
    template<class T> bool modify(const char* fn, const T& var) const
    { return use<L>::modify(fn, var) | use<R>::modify(fn,var); }
    use& set(ID3field f, std::string s)
    { use<L>::set(f,s),   use<R>::set(f,s);   return *this; }
    use& rewrite(bool t = true)
    { use<L>::rewrite(t), use<R>::rewrite(t); return *this; }
    use& create(bool t = true)
    { use<L>::create(t),  use<R>::create(t);  return *this; }
    bool from(const char* fn)
    { /* return use<L>::from(fn) | use<R>::from(fn); */
      throw "Blaugh";
         }
};

  // transform a list of parameters into a binary "type tree"
  // - fundamental cases

template<class T> struct group<T> : use<T>
{
    template<class Init> group(Init i) : use<typename group::type>(i) { }
    group() { }
};

template<class L, class R> struct group<L,R> : use< std::pair<L,R> >
{
    template<class Init> group(Init i) : use<typename group::type>(i) { }
    group() { }
};

template<class T> struct group<void,T> : use<T> { };
template<>        struct group<>       { typedef void type; };

  // generalized case
  // - e.g. group<a,b,c>::type == pair<pair<a, b>,c>

template<class T0, class T1, class T2, class T3,
         class T4, class T5, class T6, class T7>
struct group : use <
    typename group<
        typename group<
            typename group<T0,T1>::type,
            typename group<T2,T3>::type
        >::type,
        typename group<
            typename group<T4,T5>::type,
            typename group<T6,T7>::type
        >::type
    >::type
> {
    template<class Init> group(Init i) : use<typename group::type>(i) { }
    group() { }
};

  // cast function
  // - with<a>(*ptr) == ptr->use<a>::object

template<typename T> struct Rvalue     { typedef T const& type; };
template<typename T> struct Rvalue<T&> { typedef T&       type; };

template<class T> T&       with(use<T>& tree)       { return tree.object; }
template<class T>
typename Rvalue<T>::type   with(use<T> const& tree) { return tree.object; }

  // iterate over a hierarchy (const and non-const)

template<class Function, class T>
void for_each(use<T>& u, Function f)
{
    f(u.object);
}

template<class Function, class T>
void for_each(use<T> const& u, Function f)
{
    f(u.object);
}

template<class Function, class T, class U>
void for_each(use< std::pair<T,U> >& obj, Function f)
{
    use<T>& l = obj; for_each(l, f);
    use<U>& r = obj; for_each(r, f);
}

template<class Function, class T, class U>
void for_each(use< std::pair<T,U> > const& obj, Function f)
{
    use<T> const& l = obj; for_each(l, f);
    use<U> const& r = obj; for_each(r, f);
}

template<class Function, class T>
void for_each(T& group, Function f)
{
    use<typename T::type>& u = group;
    for_each(u, f);
}

template<class Function, class T>
void for_each(T const& group, Function f)
{
    use<typename T::type> const& u = group;
    for_each(u, f);
}

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // run-time delegation using standard container      //
  ///////////////////////////////////////////////////////

namespace write {

class combined : public writer, private std::vector<writer*> {
public:
  // publish some methods
    using std::vector<writer*>::size_type;
    using std::vector<writer*>::iterator;
    using std::vector<writer*>::const_iterator;
    using std::vector<writer*>::begin;
    using std::vector<writer*>::end;
    using std::vector<writer*>::size;
    using std::vector<writer*>::operator[];

  // registers tags
    combined& add(writer& object)
    { push_back(&object); return *this; }
    combined& forget(size_type pos, size_type num = 1)
    { erase(begin()+pos, begin()+pos+num); return *this; }
protected:
    bool vmodify(const char* fn, const stredit::function& f) const
    {
        bool result = false;
        for(const_iterator p = begin(); p != end(); )
            result |= (*p++)->modify(fn, f);
        return result;
    }
};

} // ::tag::write

} // ::tag

#endif

