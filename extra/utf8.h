/*

  UTF-8 (unicode transformation format) encoding/decoding layer

  copyright (c) 2003, 2004, 2005 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  The namespace 'utf8' defines a couple of things.

  utf8::length(ptr)
     - returns the code-points of a null-terminated utf8 string
  utf8::length(begin, end)
     - returns the number of code points in [begin, end)
  utf8::encode(begin, end, result)
     - encodes the code points in [begin, end) to UTF8, storing in result
  utf8::decode(begin, end, result)
     - decodes the code units in [begin, end) to UCS4, storing in result

  'ptr', 'begin' and 'end' should be Input Iterators,
  'output' should be an Output Iterator.

  UTF8 decoding is safe with respect to overlong sequences, surrogate pairs,
  and so on, and passes Markus Kuhn's UTF8 stress test:

      http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

  Types for rolling your own;

  utf8::mbbuf
     - a buffer that is large enough to hold a single multibyte character
  utf8::encoder<OutputIterator>
  utf8::decoder<OutputIterator[, ErrorFunction]>
     - encoding and decoding. see class definitions below.

  decoder takes an optional second template argument, which is a function
  pointer or function object type called for illegal input sequences,
  as if matching this signature:

      template<class utfIter> wchar error(utfIter p, wchar ucs)

  the first argument points past the offending multibyte sequence, and ucs
  is a suggested resolution: wchar(-1) for stray bytes, or the decoded value
  for overlong or otherwise invalid sequences. The default error handler is
  inline and returns U+FFFD - REPLACEMENT CHARACTER for all arguments.

  Example:

     int utf8_wctomb(char* s, wchar_t wc)
     {
         if(!s) return 0;
         utf8::encoder<char*> enc(s);
         enc.put(wc);
         return enc.base() - s;
     }

*/
#ifndef __ZF_UTF8_HPP
#define __ZF_UTF8_HPP

#if __STDC_VERSION__ >= 199901L
#  include <stdint.h>
#endif

namespace utf8 {

  // some definitions

#if __STDC_VERSION__ >= 199901L
typedef uint_fast32_t wchar;                 // ucs-4 symbol holder
#else
typedef unsigned long wchar;                 // ucs-4 symbol holder
#endif

typedef char mbbuf[6];                       // maximum symbol length

 // common internal definitions
 // - made a template class to the static table can be defined in the header

template<class T = void> class base {
public:
    template<class utfIter> struct error {
        inline wchar operator()(utfIter, wchar ucs)
        { return 0xFFFDu; }
    };
protected:
    base() { (T)void(); }                    // ensure T == void
    static inline wchar max(unsigned n);
    static unsigned char dectab[];
};

template<class utfIter>                      // invalid code handler (func)
inline wchar default_error(utfIter p, wchar ucs)
{
    return base<>::error<utfIter>()(p, ucs);
}

 // encoding and decoding primitive classes

template<class utfIter, class Error = base<>::error<utfIter> >
class decoder : base<> {
    utfIter p, end;
    Error errf;
public:
    decoder(utfIter begin, utfIter end = utfIter(), Error e = Error())
    : p(begin), end(end), errf(e) { }

    utfIter& base() { return p; }

    inline bool i_get(wchar&);
    bool get(wchar&);
};

template<class utfIter>
class encoder : base<> {
    utfIter p;
public:
    encoder(utfIter out)
    : p(out) { }

    utfIter& base() { return p; }

    inline void i_put(wchar);
    void put(wchar);
};

 // primitive functions

template<class utfIter>
unsigned long length(utfIter begin, utfIter end)

template<class utfIter>
unsigned long length(utfIter begin)

template<class utfIter, class charIter>
charIter decode(utfIter begin, utfIter end, charIter out);

template<class utfIter, class charIter>
utfIter encode(charIter begin, charIter end, utfIter out);

 // encode & decode definitions

template<class T>
inline wchar base<T>::max(unsigned n)           /* max. code of seq. n */
{
    return 1ul << n*5+1;                        /* 2 ** [ 6(n-1) + 7-n ] */
}

template<class T>
unsigned char base<T>::dectab[64] = {           /* symbol length table */
    2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,
    5,5,5,5,6,6,0,0,
};

template<class utfIter, class Error>
inline bool decoder<utfIter,Error>::i_get(wchar& out)
{
    wchar ucs;                                  /* uni. code symbol */

    wchar lng;                                  /* overlong mark */
    int seq;
    unsigned char c;

chrloop:
    if(p != end)                                /* seq == 0 loop */
        switch((c=*p++) & 0xC0) {
        default:                                /* 0x00 .. 0x7F */
            return out=c, 1;
        case 0xC0:                              /* start of sequence */
            if(seq = dectab[c & 0x3F]) {
                ucs = c & (1<<8-seq)-1;         /* mask off good bits */
                lng = max(--seq);               /* determine minimum */
                lng <<= seq==1;                 /* fix 2 byte minimum */
                goto seqloop;
            }
        case 0x80:                              /* stray byte */
            return out=errf(p, -1), 1;
        }
    return 0;

seqloop:
    while(p != end && (c=*p^0x80) <= 0x3F) {
    ++p;
        ucs = ucs << 6 | c;
        if(--seq)
            continue;

        if((ucs&~1ul    ) == 0xFFFE ||      /* illegal unicode? */
           (ucs&~0x7FFul) == 0xD800 ||      /* utf-16 surrogate? */
           (ucs < lng   )          )        /* was overlong? */
            return out=errf(p, ucs), 1;
        else
            return out=ucs, 1;
    }
    return out=errf(p, -1), 1;
}

template<class utfIter>
inline void encoder<utfIter>::i_put(wchar ucs)
{
    if(ucs < 0x80)
        *p++ = ucs;
    else {                                      /* determine bytes */
        unsigned char c;
        int n;
        ucs &= (1ul << 31) - 1;

        for(n=2; max(n) <= ucs; ) ++n;          /* could be faster? */

        for(c = 0xFF^(0xFF >> n); n--; c = 0x80)
            *p++ = c | (ucs >> n*6) & 0x3F;
    }
}

  // non-inline versions.

template<class utfIter, class Error>
bool decoder<utfIter,Error>::get(wchar& ucs)
{
    return i_get(ucs);
}

template<class utfIter>
void encoder<utfIter>::put(wchar ucs)
{
    i_put(ucs);
}

  // sequence routines

template<class utfIter>
unsigned long length(utfIter begin, utfIter end)
{
    register decoder<utfIter> utf(begin, end);
    unsigned long cnt = 0;
    for(wchar wc; utf.i_get(wc); ) ++cnt;
    return cnt;
}

template<class utfIter>
unsigned long length(utfIter begin)
{
    register decoder<utfIter> utf(begin);
    unsigned long cnt = 0;
    for(wchar wc; utf.i_get(wc), wc; ) ++cnt;
    return cnt;
}

template<class utfIter, class charIter>
charIter decode(utfIter begin, utfIter end, charIter out)
{
    register decoder<utfIter> utf(begin, end);
    wchar wc;
    while( utf.i_get(wc) )
        *out++ = wc;
    return out;
}

template<class utfIter, class charIter>
utfIter encode(charIter begin, charIter end, utfIter out)
{
    register encoder<utfIter> utf(out);
    while(begin != end)
        utf.i_put(*begin++);
    return utf.base();
}

}

#endif

/*

 UTF-8, terse
 Any byte with bit7 clear is that byte.

 Any byte with bit7 belongs to a sequence of bytes.
   All but the first have bit6 clear, the remaining 6bits carrying data.

   The starting byte consists of a number of bits (counted from msb side),
   which indicate the number of bytes in this sequence (at least two),
   followed by a zero bit, followed by a few bits of data.

*/


