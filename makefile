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
.PHONY: dist dist-zip dist-clean dist-check diff fetch-orig

.SUFFIXES: .c .cpp .o

## standard targets ########################################################

build: id3

all  : id3 id3l

final: id3 id3l
	$(STRIP) id3 id3l

clean:
	-rm -f *.o id3 id3l

## installation ############################################################

installdirs:
	$(INSTALL_DIR) $(bindir) $(man1dir)

installman: id3.man
	$(INSTALL_DATA) id3.man $(man1dir)/id3.$(manext)

installdoc: $(docdata)
	$(INSTALL_DIR) $(docdir)
	for f in $(docdata); do \
	    $(INSTALL_DATA) $${f} $(docdir)/$${f}; done

install: $(binary) installdirs installman
	$(INSTALL) $(binary) $(bindir)/id3

install-strip: $(binary) installdirs installman
	$(INSTALL_STRIP) $(binary) $(bindir)/id3

install-full: installdoc install-strip

uninstall:
	-rm -f $(man1dir)/id3.$(manext)
	-rm -f $(bindir)/id3
	-for f in $(docdata); do \
	    rm -f $(docdir)/$${f}; done
	-rmdir $(docdir)

## distribution ############################################################

DISTFILES = INSTALL $(docdata) makefile makefile.dj makefile.bcc \
	main.cpp auto_dir.h \
	$(foreach f, sedit varexp ffindexp charconv, $(f).h $(f).cpp) \
	$(foreach f, set_base setid3 setid3v2 setfname, $(f).h $(f).cpp) \
	$(foreach f, fileops id3v1 id3v2, $(f).h $(f).c) \
	id3.man \
	$(foreach f, control rules copyright changelog, debian/$(f))

D_VER = `sed -n "/_version_/{s:[^0-9]*\([^ ]*\).*:\1:p;q;}" main.cpp`

D_PKG = pkg=id3-$(D_VER); \
	rm -f $${pkg}; \
	ln -s `pwd` $${pkg}

D_FIL = `echo $(DISTFILES) | sed "s:[^ ]*:$${pkg}/&:g"`

D_TMP = rm -rf .tmp; mkdir .tmp && \
	tar c $(DISTFILES) | tar xC .tmp

dist: $(DISTFILES)
	$(D_PKG) && $(TAR) chofz $${pkg}.tar.gz $(D_FIL); rm -f $${pkg}

dist-zip: $(DISTFILES)
	$(D_PKG) && zip -9Tl $${pkg//.}s.zip $(D_FIL); rm -f $${pkg}

dist-check:
	if [ -f !* ]; then echo !*; false; fi
	grep $(D_VER) INSTALL
	grep $(D_VER) CHANGES
	d=`(cat INSTALL | sed -n -e '/contents/,/---/!d' \
	  -e '/(C++)$$/{ s/^ \([^ ]*\)[*].*/\1h/p; s/h$$/cpp/p;}' \
	  -e '/(C)$$/  { s/^ \([^ ]*\)[*].*/\1h/p; s/h$$/c/p;  }' \
	  -e		's/^ \([^ ]*\).*/\1/p'; \
	    ls $(DISTFILES) | sed 's:/[^ ]*::g') | sort | uniq -u` && \
	echo "$${d}"; test -z "$${d}"
	$(D_TMP) && make -C .tmp all && mv .tmp/id3 .tmp/id3l `pwd`
	-rm -rf .tmp
	@echo all release checks okay

dist-clean:
	$(D_TMP)
	-rm -rf *
	mv .tmp/* `pwd`
	-rm -rf .tmp

diff:
	rm -rf .tmp; mkdir .tmp && ln -s `pwd` .tmp/{current}
	tar Cxfz .tmp `pwd`.tar.gz
	diff -x '.*' -durN .tmp/* | gzip -9 > `pwd`-$(D_VER).diff.gz
	-rm -rf .tmp

URL_PREFIX=http://home.wanadoo.nl/squell

fetch-orig:
	wget $(URL_PREFIX)/`wget -q -O - $(URL_PREFIX)/id3.html | \
	  sed -n 's/^.*href="\([[:alnum:]/_.-]*[.]tar[.]gz\)".*$$/\1/p'`

## build rules #############################################################

OBJ_GEN = sedit.o varexp.o ffindexp.o charconv.o
OBJ_1	= setid3.o id3v1.o
OBJ_2	= setid3v2.o id3v2.o fileops.o
OBJ_F	= setfname.o
OBJECTS = main.o $(OBJ_GEN) set_base.o $(OBJ_1) $(OBJ_2) $(OBJ_F)

id3: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)

id3l: mainl.o $(OBJ_GEN) $(OBJ_1)
	$(CXX) $(LDFLAGS) -o $@ mainl.o $(OBJ_GEN) set_base.o $(OBJ_1) $(OBJ_F)

.cpp.o:
	$(CC) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CFLAGS) -c $<

mainl.o: main.cpp ffindexp.h auto_dir.h sedit.h set_base.h setid3.h \
  setfname.h
	$(CC) $(CXXFLAGS) -DNO_V2 -o $@ -c main.cpp

## dependencies -MM ########################################################

fileops.o: fileops.c fileops.h
id3v1.o: id3v1.c id3v1.h
id3v2.o: id3v2.c fileops.h id3v2.h
charconv.o: charconv.cpp charconv.h
ffindexp.o: ffindexp.cpp varexp.h auto_dir.h ffindexp.h
main.o: main.cpp ffindexp.h auto_dir.h sedit.h set_base.h setid3.h \
  setfname.h setid3v2.h
sedit.o: sedit.cpp charconv.h sedit.h
set_base.o: set_base.cpp set_base.h sedit.h
setfname.o: setfname.cpp setfname.h set_base.h sedit.h
setid3.o: setid3.cpp setid3.h set_base.h sedit.h id3v1.h
setid3v2.o: setid3v2.cpp setid3v2.h set_base.h sedit.h id3v1.h id3v2.h \
  fileops.h
varexp.o: varexp.cpp varexp.h

############################################################################
