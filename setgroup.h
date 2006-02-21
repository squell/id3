#ifndef __ZF_SETGROUP_HPP
#define __ZF_SETGROUP_HPP

#include <utility>
#include "set_base.h"

namespace tag {

  ///////////////////////////////////////////////////////
  // generic implementation                            //
  // compile time delegation - template metaprograms   //
  ///////////////////////////////////////////////////////

template<class T0 = void, class T1 = void, class T2 = void, class T3 = void,
         class T4 = void, class T5 = void, class T6 = void, class T7 = void>
struct group;

  // encapsulate a type, and control the virtual signatures it introduces

struct nihil { };

template<class T, class Interface = nihil>
struct use : Interface {
    typedef T type, dynamic;
    dynamic object;

    use(type init = type()) : object(init) { }

    bool modify(const char* fn) const        // non-virtual pass-throughs
    { return object.modify(fn); }
    template<class P> bool modify(const char* fn, const P& var) const
    { return object.modify(fn, var); }
    use& set(ID3field f, std::string s)
    { return object.set(f,s),   *this; }
    use& rewrite(bool t = true)
    { return object.rewrite(t), *this; }
    use& create(bool t = true)
    { return object.create(t),  *this; }
private:
    bool vmodify(const char* fn, const stredit::function& f) const
    { return object.modify(fn, f);  }
};

  // transforms a recursively nested type into a boxed-type hierarchy
  // - on a decent compiler, this will only emit code if ::dynamic is used
  // - passing only "messages" through this would be better, but creating
  // - those messages is involved as well

template<class L, class R>
struct use< std::pair<L,R> > : use<L>, use<R> {
    typedef std::pair<L, R> type;
    typedef use<use, handler> dynamic;

    template<class U> use(U& init) : use<L>(init), use<R>(init) { }
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
    bool from(const char* fn) const
    { return use<L>::from(fn) | use<R>::from(fn); }
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
template<> struct group<> { typedef void type; };

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
  // - with<a>(*ptr) == ref->use<a>::object

template<typename T> struct Rvalue     { typedef T const& type; };
template<typename T> struct Rvalue<T&> { typedef T&       type; };

template<class T> T&       with(use<T>& tree)       { return tree.object; }
template<class T>
typename Rvalue<T>::type   with(use<T> const& tree) { return tree.object; }

  // iterate over a hierarchy

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

class combined : public handler, private std::vector<handler*> {
public:
  // registers tags
    combined& add(handler& object)
    { push_back(&object); return *this; }
    combined& forget(size_type pos, size_type num = 1)
    { erase(begin()+pos, begin()+pos+num); return *this; }

  // standard state set methods
    combined& rewrite(bool t = true);
    combined& create(bool t = true);
    combined& set(ID3field i, std::string m);
    bool from(const char*);

    bool vmodify(const char*, const stredit::function&) const;

  // publish some methods
    using std::vector<handler*>::iterator;
    using std::vector<handler*>::begin;
    using std::vector<handler*>::end;
    using std::vector<handler*>::size;
    using std::vector<handler*>::operator[];
};

combined& combined::rewrite(bool t)
{
    for(iterator p = begin(); p != end(); (*p++)->rewrite(t));
    return *this;
}

combined& combined::create(bool t)
{
    for(iterator p = begin(); p != end(); ) (*p++)->create(t);
    return *this;
}

combined& combined::set(ID3field i, std::string m)
{
    for(iterator p = begin(); p != end(); ) (*p++)->set(i,m);
    return *this;
}

bool combined::from(const char* fn)
{
    bool result = false;
    for(iterator p = begin(); p != end(); ) result |= (*p++)->from(fn);
    return result;
}

bool combined::vmodify(const char* fn, const function& f) const
{
    bool result = false;
    for(const_iterator p = begin(); p != end(); )
        result |= (*p++)->modify(fn, f);
    return result;
}

}

#endif

