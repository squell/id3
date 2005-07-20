/*

  verbose_t class (main.cpp include)

  (c) 2005 squell ^ zero functionality!
  see the file 'COPYING' for license conditions

*/

struct verbose_t {
    bool          show;
    clock_t       time;
    unsigned long numfiles;
    const char*   dir;

    verbose_t()
    : show(false), dir(0), time(clock()), numfiles(0) { }
    void on()
    { show = true; }

   ~verbose_t()
    {
        time = clock() - time;
        if(show) {
            if(exitc!=0) fprintf(stderr, "Errors were encountered\n");
            if(exitc!=1) fprintf(stderr, "(%lu files in %.3fs) done\n", numfiles, double(time) / CLOCKS_PER_SEC);
        }
    }

    void reportd(const char* s)     // reporting a dir
    {
        dir = s;
    }

    void reportf(const char* s)     // reporting a filename
    {
        if(show) {
            ++numfiles;
            if(dir && s-dir) fprintf(stderr, "%.*s\n", s-dir, dir);
            if(*s)           fprintf(stderr, "\t%s\n", s);
            dir = 0;
        }
    }

} static verbose;

