#include "varexp.h"
#include <vector>
#include <cstring>

/*

  (c) 2003 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  an adapted version of a smaller, simpler wildcard match routine to
  support a primitive form of pattern matching

  the base C version included at the end, if you're interested

*/

bool varexp::match(const char* mask, const char* test)
{
    std::vector<const char*> _vars = vars;     // backup previous values
    std::vector<int>         _varl = varl;     //
    bool flag = false;

    char m, c;
    while( m=*mask++, c=*test++, m && c ) {
        if(m=='*') {
            if(!flag) {
                vars.push_back(test-1);   // add entry for new variable
                varl.push_back(0);        // + length count
                flag = true;
            }
            if( match(mask,test-1) || (++varl.back(),match(mask,test)) )
                return 1;
            else
                --mask;
        } else {
            flag = false;
            if(m!='?' && m!=c)
                break;                    // bool(c|m) == true. (*)
        }
    }
    if(c|m) {                             // (*)
       vars = _vars;
       varl = _varl;
       return 0;
    }
    return 1;
}

/*

 C version without pattern matching (more concise):
 (ok, so it's dense - big deal)

int match(const char *mask, const char *test)
{
    char m, c;
    while( m=*mask++, c=*test++, m && c ) {
        if(m=='*')
            if( match(mask,test-1) || match(mask,test) )
                return 1;
            else
                --mask;
        else if(m!='?' && m!=c)
            return 0;
    }
    return(!(c|m))
}

*/

