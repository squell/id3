/*

  verbose_t class (main.cpp include)

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

struct verbose_t {
    bool          show;
    clock_t       time;
    unsigned long numfiles;

    verbose_t()
    : show(false), time(clock()), numfiles(0) { }
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
            fprintf(stderr, "\t%s\n", s);
        }
    }

    void reportd(const char* s)                     // reporting a dir
    {
        if(show && *s) fprintf(stderr, "%s\n", s);
    }
} static verbose;

