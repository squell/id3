/*

  character conversion (user locale -> latin1)

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  this is a placeholder

  the function pointed to by conv converts it's argument to ISO Latin1
  encoding, if it can.

*/

#include <string>

namespace latin1 {

    extern std::string (*conv)(const std::string&);  // set on initialization
    extern const char* charset();                    // returns locale or NULL

    extern std::string fallback(const std::string&);

}

