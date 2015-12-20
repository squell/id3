/*

  serializing tag info to/from human-readable text

  copyright (c) 2015 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

      output(tags, filename, out)

  Output the selected tags (a vector) contained in fn to the output file

      output(begin, end, out)

  Output the selected range of frames from a metadata::listing


  Output format:

  A simple a textfile in two logical columns (seperated by tabs):

  1) the first column contains "keys" whereas the
  2) second column contains data pertaining to the most recent key

  Output can come in two forms:
  1) key<tab>value
        a simple key-value pair, with value not containing newlines

  2) key<newline><tab>value1[<newline><tab>value2 ... <newline><tab>valueN]

        in this case value is the concatenation of the value1...valueN pairs,
        joined together by newlines (as in Python's .join function)

  Keys come in two forms: {FIELD}'s that mention metadata fields (actual data
  read from a tags), and #directive's (information about the file/tags).

*/

#ifndef DUMPTAG_H
#define DUMPTAG_H

#include <cstdio>
#include "set_base.h"
#include "setgroup.h"

namespace tag {

    void output(combined<reader> const& tags, const char* filename, std::FILE* out);
    void output(metadata::array::const_iterator begin, metadata::array::const_iterator end, std::FILE* out);

}

#endif
