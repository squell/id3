/*

  verbose_t class

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

#ifndef __ZF_VERBOSE_HPP
#define __ZF_VERBOSE_HPP

struct verbose_t {
    int           exitc;
    bool          show;
    clock_t       time;
    unsigned long numfiles;

    verbose_t() : show(false), time(clock()), numfiles(0), exitc(0)
    { }
    void on()
    { show = true; }

   ~verbose_t()
    {
        time = clock() - time;
        if(show) {
            if(exitc!=0) fprintf(stderr, "Errors were encountered\n");
            if(exitc!=1) fprintf(stderr, "(%d files in %.3fs) done\n", numfiles, double(time) / CLOCKS_PER_SEC);
        }
    }

    void reportf(const char* s)                     // reporting a filename
    {
        if(show) {
            ++numfiles;
            const char* sep = strrchr(s, '/');
            fprintf(stderr, "\t%s\n", sep?sep+1:s);
        }
    }

    void reportd(const char* s)                     // reporting a dir
    {
        if(show && *s) fprintf(stderr, "%s\n", s);
    }
};


#endif

