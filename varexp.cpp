#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "varexp.h"

/*

  copyright (c) 2004, 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  an adapted version of a smaller, simpler wildcard match routine to
  support a primitive form of pattern matching

  the base C version included at the end, if you're interested

*/

bool varexp::match(const char *mask, const char *test)
{
    char c, m;
    const char *flag = &c;                            // dummy value
    do {
        const char* resume[2] = { flag, };
        do {
retry:      switch( c=*test++, m=*mask++ ) {
            default :
                if( m != c ) goto fail;               // double break
                continue;
            case '?':
                int skip; skip = mblen(test-1, MB_CUR_MAX);
                if(skip > 1) test += skip-1;
                continue;
            case '*':
                if(*resume)
                    flag = 0, var.push_back(pairvec::value_type(test-1, 0));
                else
                    var.back().second++;
                resume[0] = mask-1, resume[1] = test;
                --test;
                goto retry;                           // forced continue
            case '[':
                if(! (mask=in_set(c, mask)) ) goto fail;
            }
        } while(c);
fail:   mask = resume[0];
        test = resume[1];
    } while(c && test);
    return !(m|c);
}

 /*
     auxilliary code to detect character ranges in expressions
     uses plain value comparison to detect ranges; not collating sequences.

     issue: refuses to compare non-ascii characters in order to avoid
     serious problems with UTF-8 strings; varexp should really be lifted
     to the wchar_t world.
 */

const char *varexp::in_set(char c, const char *set)
{
    const char *start = set;
    int  neg = 0, truth = 0;
    char prev, m;
    if(*set=='!' || *set=='^') {
        neg = 1;                       // match chars NOT in set
        ++set;
    }
    for(prev = 0; (m = *set++) && !(m & 0x80); prev = m) {
        if(m=='-' && prev && *set!='\0' && *set!=']') {
            truth |= (c >= prev) && (c <= *set);
        } else {
            if(m==']')
                return (truth^neg)? set : 0;
            truth |= (m==c);
        }
    }
    return (c == '[')? start : 0;      // not a set notation
}

/*

 primitive C version without pattern matching (inefficient)

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

 non-recursive C version without pattern matching

int match(const char *mask, const char *test)
{
    char c, m;
    do {
        const char* resume[2] = { 0, };
        do {
retry:      switch( c=*test++, m=*mask++ ) {
            default :
                if( m != c ) goto fail;               // double break
            case '?':
                continue;
            case '*':
                resume[0] = mask-1, resume[1] = test;
                --test;
                goto retry;                           // forced continue
            }
        } while(c);
fail:   mask = resume[0];
        test = resume[1];
    } while(c && mask);
    return !(m|c);
}

*/

