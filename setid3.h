/*

  set_tag::ID3 applicative class

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  Usage:

  The set_tag::ID3 class implements the single_tag interface for ID3 tags

  Example:

  int main(int argc, char* argv[])
  {
	  set_tag::ID3()
	  .set(artist, "%2")
	  .set(title,  "%3")
	  .modify(argv[1], argv);
  }

*/

#ifndef __ZF_SETID3
#define __ZF_SETID3

#include <string>
#include <vector>
#include "set_base.h"

namespace set_tag {

class ID3 : public single_tag {
	std::vector<const char*> mod;		   // modification data
public:
    ID3(bool f = true) : mod(FIELDS,(char*)0), single_tag(f) { }

    virtual int vmodify(const char*, const base_container&) const;

  // standard set

	ID3& set(ID3field i, const char* m) 	// set ID3 field i to value m
	{ if(i < FIELDS) mod[i] = m; return *this; }
};

}

#endif

