/*

  Lyrics3 input/output

  copyright (c) 2006 squell <squell@alumina.nl>

  use, modification, copying and distribution of this software is permitted
  under the conditions described in the file 'COPYING'.

  Usage:

  - lyrics3::field(id, content)
  Returns a Lyrics3 'ID' field containing 'content', or empty if invalid

  - lyrics3::find(tag, id)
  Returns the content of the 'ID' field of 'tag', or empty if not found

  - lyrics3::read(file[, id3v1buf])
  Returns the Lyrics3 tag of 'file', or empty if none. if id3v1buf != NULL,
  stores a found id3v1 tag there, or a null byte if none.

  - lyrics3::cast(string)
  IF string is a valid Lyrics3 tag, returns it, otherwise empty

  - lyrics3::write(file, tag[, id3v1buf])
  Writes the Lyrics3 'tag' to 'file'. Returns 0 on success, non-zero on
  failure, and negative in case of a serious write error.

  - lyrics3::find_next(tag, cur[, id3v1buf])
  Returns the end position of a Lyrics3 frame in 'tag', or 0 if already ended

  - lyrics3::info
  Restricted string. Added members: str(), id(i), content(i[,next])

  Example:

    lyrics3::info tag = lyrics3::read("foo.mp3");
    for(long n, i = 0; n=lyrics3::find_next(tag,i); i = n)
        std::cout << tag.id(i) << ": " << tag.content(i,n) << std::endl;

*/
#ifndef __ZF_LYRICS3_HPP
#define __ZF_LYRICS3_HPP

#include <string>
#include <iterator>

namespace lyrics3 {

    class info : std::string {
    public:
        friend info field(const std::string& id,  const std::string& content);
        friend info cast (const std::string&);
        friend info read (const char* fn, void* id3);
        friend int  write(const char* fn, const info& tag, const void* newid3);

        friend std::string     find     (const info& s, const std::string& sig);
        friend info::size_type find_next(const info& s, info::size_type pos);
        friend info field(const std::string& s);

        info& operator+=(const info& other)
        { return std::string::operator+=(other), *this; }
        friend info operator+(info lhs, const info& rhs)
        { return lhs += rhs; }

        const std::string& str() const { return *this; }
        using std::string::size;
        using std::string::c_str;

        std::string id(size_type i) const
        { return substr(i, 3); }
        std::string content(size_type i, size_type n) const
        { return substr(i+8, n-i-8); }
        std::string content(size_type i) const
        { return substr(i+8, find_next(*this, i)-i-8); }
        info() { }

        class iterator;
    private:
        info(const std::string& s) : std::string(s) { }
    };

    info field(const std::string& id,  const std::string& content);
    info cast (const std::string&);
    info read (const char* fn, void* id3 = 0);
    int  write(const char* fn, const info& tag, const void* newid3 = 0);

    std::string     find     (const info& s, const std::string& sig);
    info::size_type find_next(const info& s, info::size_type pos);
    info field(const std::string& s);
}

#endif

