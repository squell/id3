#include <cstdio>
#include <iostream>
#include <string>
#include "id3v2.h"

class ID3tag {
public:
    class iterator;

    ID3tag(const char* fname);
    ~ID3tag();

    unsigned long size();

    iterator begin();
    iterator end();

private:
    unsigned long tagsize;
    void* buf;
};

class ID3tag::iterator {
    friend class ID3tag;
public:
    class frame {
        friend class iterator;
        ID3FRAME f;
    public:
        int tag_volit()    const { return f->tag_volit;  }
        int file_volit()   const { return f->file_volit; }
        int readonly()     const { return f->readonly;   }
        int packed()       const { return f->packed;     }
        int encrypted()    const { return f->encrypted;  }
        int grouped()      const { return f->grouped;    }
        const char* ID()   const { return f->ID;         }
        std::string data() const { return std::string(f->data, f->size); }
    };

    iterator() : stat(false) { }

    iterator& operator++()
    { stat = ID3_frame(f.f); return *this; }

    iterator  operator++(int)
    { iterator t = *this; ++(*this); return t; }

    const frame& operator*() const
    { return frm; }

    const frame* operator->() const
    { return &frm; }

    bool operator==(const iterator& other) const
    { return stat == other.stat && (!stat || f.f->data == other.f.f->data); }

    bool operator!=(const iterator& other) const
    { return !(*this == other); }

    operator bool()
    { return stat; }

private:
    frame f;
    bool stat;

    iterator(void* buf)
    { ID3_start(f.f, buf); stat = ID3_frame(f.f); }
};

inline ID3tag::iterator ID3tag::begin()
{
    if(buf)
        return iterator(buf);
    else
        return end();
}

inline ID3tag::iterator ID3tag::end()
{ return iterator(); }

inline ID3tag::ID3tag(const char* fname)
{ buf = ID3_readf(fname, &tagsize); }

inline ID3tag::~ID3tag()
{ ID3_free(buf); }

inline unsigned long ID3tag::size()
{ return tagsize; }

int main(int argc, char *argv[])
{
    ID3tag tag(argv[1]);

    for(ID3tag::iterator p = tag.begin(); p != tag.end(); p++)
//      std::printf("%s\n\t%.*s\n", p->ID, (int)p->size-1, p->data+1);
        cout << p->ID() << endl << p->data() << endl;
}

