## generic GNU makefile ####################################################

SHELL  = /bin/sh

CC       = gcc
CXX      = g++
CFLAGS   = -g -O2
CXXFLAGS = $(CFLAGS)
LDFLAGS  =

STRIP    = strip

## installation vars #######################################################

prefix   = /usr/local
bindir   = $(prefix)/bin
datadir  = $(prefix)/share
mandir   = $(prefix)/man
manext   = 1

docdir   = $(datadir)/doc/id3
man1dir  = $(mandir)/man$(manext)

binary   = id3
docdata  = README CHANGES COPYING

INSTALL       = install
INSTALL_DIR   = $(INSTALL) -d
INSTALL_STRIP = $(INSTALL) -s
INSTALL_DATA  = $(INSTALL) -m 644

############################################################################

id3: main.o sedit.o varexp.o ffindexp.o \
     setid3.o setid3v2.o \
     id3v1.o id3v2.o fileops.o
	$(CXX) $(LDFLAGS) -o $@ $+

id3l: mainl.o sedit.o varexp.o ffindexp.o \
      setid3.o \
      id3v1.o
	$(CXX) $(LDFLAGS) -o $@ $+

all  : id3 id3l

final: id3 id3l
	$(STRIP) $+

clean:
	rm -f *.o id3 id3l

############################################################################

installdirs:
        $(INSTALL_DIR) $(bindir) $(man1dir) $(docdir)

installman: id3.man README
        $(INSTALL_DATA) id3.man $(man1dir)/id3.$(manext)
	for f in $(docdata); do  \
	    $(INSTALL_DATA) $${f} $(docdir)/$${f}; done

install: $(binary) installdirs installman
	$(INSTALL) $(binary) $(bindir)/id3

install-strip: $(binary) installdirs installman
	$(INSTALL_STRIP) $(binary) $(bindir)/id3

uninstall:
        rm -f $(man1dir)/id3.$(manext)
	rm -f $(bindir)/id3
	for f in $(docdata); do \
	    rm -f $(docdir)/$${f}; done

############################################################################

main.o: main.cpp sedit.h ffindexp.h auto_dir.h setid3v2.h setid3.h
        $(CC) $(CXXFLAGS) -c main.cpp

mainl.o: main.cpp sedit.h ffindexp.h auto_dir.h setid3.h
        $(CC) $(CXXFLAGS) -DNO_V2 -o $@ -c main.cpp

ffindexp.o: varexp.h auto_dir.h
setid3.o  : sedit.h id3v1.h
setid3v2.o: setid3.h sedit.h id3v1.h id3v2.h fileops.h
id3v2.o   : fileops.h

%.o : %.cpp %.h
        $(CC) $(CXXFLAGS) -c $<

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<

############################################################################

