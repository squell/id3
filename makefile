## generic GNU makefile ####################################################

SHELL	 = /bin/sh

CC	 = gcc
CXX	 = g++
CFLAGS	 = -g -O2
CXXFLAGS = $(CFLAGS) -fno-rtti
LDFLAGS  =

STRIP	 = strip
TAR	 = tar

## installation vars #######################################################

prefix	 = /usr/local
bindir	 = $(prefix)/bin
mandir	 = $(prefix)/man
manext	 = 1
man1dir  = $(mandir)/man$(manext)

binary	 = id3

datadir  = $(prefix)/share
docdir	 = $(datadir)/doc/id3
docdata  = README CHANGES COPYING

INSTALL       = install
INSTALL_DIR   = $(INSTALL) -d
INSTALL_STRIP = $(INSTALL) -s
INSTALL_DATA  = $(INSTALL) -m 644

## makefile setup ##########################################################

.PHONY: all clean final default
.PHONY: install install-strip install-full uninstall
.PHONY: installdirs installman installdoc
.PHONY: dist dist-zip

.SUFFIXES: .c .cpp .o

## standard targets ########################################################

default: id3

all  : id3 id3l

final: id3 id3l
	$(STRIP) id3 id3l

clean:
	rm -f *.o id3 id3l

## installation ############################################################

installdirs:
	$(INSTALL_DIR) $(bindir) $(man1dir)

installman: id3.man
	-$(INSTALL_DATA) id3.man $(man1dir)/id3.$(manext)

installdoc: $(docdata)
	$(INSTALL_DIR) $(docdir)
	for f in $(docdata); do  \
	    $(INSTALL_DATA) $${f} $(docdir)/$${f}; done

install: $(binary) installdirs installman
	$(INSTALL) $(binary) $(bindir)/id3

install-strip: $(binary) installdirs installman
	$(INSTALL_STRIP) $(binary) $(bindir)/id3

install-full: installdoc install-strip

uninstall:
	rm -f $(man1dir)/id3.$(manext)
	rm -f $(bindir)/id3

## distribution ############################################################

DISTFILES = INSTALL $(docdata) makefile makefile.dj makefile.bcc \
	main.cpp auto_dir.h \
	$(foreach f, sedit varexp ffindexp charconv, $(f).h $(f).cpp) \
	$(foreach f, set_base setid3 setid3v2 setfname, $(f).h $(f).cpp) \
	$(foreach f, fileops id3v1 id3v2, $(f).h $(f).c) \
	id3.man

D_PKG = pkg=id3-`sed -n -e "/_version_/{s:^[^0-9]*\([^ ]*\).*$$:\1:p;q;}" \
		main.cpp`; \
	rm -f $${pkg}; \
	ln -s `pwd` $${pkg}

D_FIL = `echo $(DISTFILES) | sed -e "s:[^ ]*:$${pkg}/&:g"`

dist: $(DISTFILES)
	$(D_PKG); $(TAR) cfhz $${pkg}.tar.gz $(D_FIL); rm -f $${pkg}

dist-zip: $(DISTFILES)
	$(D_PKG); zip -9l $${pkg//.}s.zip $(D_FIL); rm -f $${pkg}

## build rules #############################################################

OBJ_GEN = sedit.o varexp.o ffindexp.o charconv.o
OBJ_1	= setid3.o id3v1.o
OBJ_2	= setid3v2.o id3v2.o fileops.o
OBJ_F	= setfname.o
OBJECTS = main.o $(OBJ_GEN) set_base.o $(OBJ_1) $(OBJ_2)

id3: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)

id3l: mainl.o $(OBJ_GEN) $(OBJ_1)
	$(CXX) $(LDFLAGS) -o $@ mainl.o $(OBJ_GEN) set_base.o $(OBJ_1)

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CFLAGS) -c $<

mainl.o: main.cpp ffindexp.h auto_dir.h set_base.h sedit.h setid3.h
	$(CC) $(CXXFLAGS) -DNO_V2 -o $@ -c main.cpp

## dependencies -MM ########################################################

fileops.o: fileops.c fileops.h
id3v1.o: id3v1.c id3v1.h
id3v2.o: id3v2.c fileops.h id3v2.h
charconv.o: charconv.cpp charconv.h
ffindexp.o: ffindexp.cpp varexp.h auto_dir.h ffindexp.h
main.o: main.cpp ffindexp.h auto_dir.h set_base.h sedit.h setid3.h \
  setid3v2.h
sedit.o: sedit.cpp charconv.h sedit.h
set_base.o: set_base.cpp set_base.h sedit.h
setid3.o: setid3.cpp setid3.h set_base.h sedit.h id3v1.h
setid3v2.o: setid3v2.cpp setid3v2.h set_base.h sedit.h id3v1.h id3v2.h \
  fileops.h
varexp.o: varexp.cpp varexp.h

############################################################################

