/*

  ID3v1.1 structures, genres 

  Made this into a header + extern const object, lest certain linkers start
  emitting multiple copies in executables. And if i'm going to use a header
  for the genres, why not put the ID3v1 tag structure here as well, eh?

*/

#ifndef __ZF_ID3V1_H
#define __ZF_ID3V1_H

extern const char *const ID3v1_genres[];

struct ID3v1 {                                   // ID3 v1.1 tag structure
    char TAG   [3];
    char title [30];
    char artist[30];
    char album [30];
    char year  [4];
    char cmnt  [28];
    char __;
    unsigned char track;
    unsigned char genre;
};

#endif

