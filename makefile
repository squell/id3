## generic GNU makefile ####################################################

SHELL  = /bin/sh

CC     = gcc
CXX    = g++
CFLAGS = -g -O2

STRIP  = strip

## installation vars #######################################################

prefix = /usr/local
bindir = $(prefix)/bin

binary = id3

INSTALL       = install
INSTALL_DIR   = $(INSTALL) -d
INSTALL_STRIP = $(INSTALL) -s

############################################################################

id3: main.o varexp.o setid3.o setid3v2.o id3v1.o id3v2.o fileops.o
	$(CXX) -o $@ $+

id3l: mainl.o varexp.o setid3.o id3v1.o
	$(CXX) -o $@ $+

all  : id3 id3l

final: id3 id3l
	$(STRIP) $+

clean:
	rm *.o id3 id3l

############################################################################

installdirs:
	$(INSTALL_DIR) $(bindir)

install: installdirs $(binary)
	$(INSTALL) $(binary) $(bindir)/id3

install-strip: installdirs $(binary)
	$(INSTALL_STRIP) $(binary) $(bindir)/id3

uninstall:
	rm -f $(bindir)/id3

############################################################################

main.o: main.cpp varexp.h setid3.h setid3v2.h id3v1.h id3v2.h
	$(CC) $(CFLAGS) -c main.cpp

mainl.o: main.cpp varexp.h setid3.h id3v1.h
	$(CC) $(CFLAGS) -DNO_V2 -o $@ -c main.cpp

setid3v2.o: id3v2.h setid3.h fileops.h
setid3.o  : id3v1.h
id3v2.o   : fileops.h

%.o : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<

############################################################################

