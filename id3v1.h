/*

  ID3v1.1 structures, genres

  copyright (c) 2003 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Made this into a header + extern const object, lest certain linkers emit
  multiple copies in executables. And if i'm going to use a header for the
  genres, why not put the ID3v1 tag structure in here as well, eh?

*/

#ifndef __ZF_ID3V1_H
#define __ZF_ID3V1_H

#ifdef __cplusplus
extern "C" {
#endif

extern const char         *const ID3v1_genre[];
extern       unsigned char const ID3v1_numgenres;

struct ID3v1 {                                   /* ID3 v1.1 tag structure */
    char TAG   [3];
    char title [30];
    char artist[30];
    char album [30];
    char year  [4];
    char cmnt  [28];
    char zero;
    unsigned char track;
    unsigned char genre;
};

typedef int ID3v1_is_128bytes_check [sizeof(struct ID3v1)==128 ? 1 : -1];

#ifdef __cplusplus
}
#endif
#endif
