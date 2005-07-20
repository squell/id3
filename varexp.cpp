#include "varexp.h"

/*

  (c) 2004 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

  an adapted version of a smaller, simpler wildcard match routine to
  support a primitive form of pattern matching

  the base C version included at the end, if you're interested

*/

bool varexp::match(const char* mask, const char* test)
{
    pairvec::size_type prevlen = var.size();   // backup value
    bool flag = false;

    char m, c;
    do {
        switch( c=*test++, m=*mask++ ) {
        case '[':
            if( in_set(c,mask,test) ) return 1;
        default :
            if( m != c ) return 0;
        case '?':
            flag = false;
            break;

        case '*':
            if(!flag) {                   // add entry for new variable
                var.push_back(pairvec::value_type(test-1, 0));
                flag = true;
            }
            if( match(mask,test-1) ) return 1;
            --mask, ++var.back().second;
        }
    } while(c);

    if(m != '\0') {
        var.resize(prevlen);
        return 0;
    }
    return 1;
}

 /*
     auxilliary code to detect character ranges in expressions
     uses plain value comparison to detect ranges
 */

int varexp::in_set(char c, const char* set, const char* rest)
{
    int  neg = 0, truth = 0;
    char prev, m;
    if(*set=='!' || *set=='^') {
        neg = 1;                       // match chars NOT in set
        ++set;
    }
    for(prev = 0; (m = *set++); prev = m)
        if(m=='-' && prev && *set!='\0' && *set!=']') {
            truth |= (c >= prev) && (c <= *set);
        } else {
            if(m==']')
                return (truth^neg) && match(set, rest);
            truth |= (m==c);
        }
    return 0;
}

/*

 C version without pattern matching (more concise):

int match(const char *mask, const char *test)
{
    char c, m;
    do {
        switch( c=*test++, m=*mask++ ) {
        default :
            if( m != c ) return 0;
        case '?':
            break;
        case '*':
            if( match(mask,test-1) ) return 1;
            --mask;
        }
    } while(c);
    return !m;
}

*/

