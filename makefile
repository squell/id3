## generic GNU makefile ####################################################

SHELL	 = /bin/sh

CC	 = gcc
CXX	 = g++
CFLAGS	 = -g -O2
CXXFLAGS = $(CFLAGS) -fno-rtti
LDFLAGS  =

STRIP	 = strip
TAR	 = tar
MKDEP	 = $(CC) -MM

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

INSTALL 	= install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA	= $(INSTALL) -m 644
INSTALL_DIR	= $(INSTALL) -d
INSTALL_STRIP	= $(INSTALL_PROGRAM) -s

## makefile setup ##########################################################

.PHONY: build all final clean depend help
.PHONY: install install-strip install-full uninstall
.PHONY: installdirs installman installdoc
.PHONY: dist dist-zip dist-clean dist-check diff
.PHONY: wget-orig fetch-orig curl-orig

.SUFFIXES: .c .cpp .o

## standard targets ########################################################

build: id3

all  : id3 id3l

final: id3 id3l
	$(STRIP) id3 id3l

clean:
	-rm -f *.o id3 id3l

depend:
	(echo '/$@encies/+2;/^$$/-1c'; $(MKALLDEP); \
	 echo .; echo wq) | ed makefile

help:
	@sed -e '1,/vars/!d' -e '/^[[:upper:]]/!d' makefile
	@echo
	@sed -e '1,/setup/!d' -e '/^[[:lower:]]/!d' makefile
	@echo
	@grep '^.PHONY' makefile

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

SRC_CPP    += sedit varexp ffindexp charconv
SRC_CPP    += set_base setid3 setid3v2 setfname
SRC_C	   += fileops id3v1 id3v2
DIR_DEBIAN += control rules copyright changelog

DISTFILES = INSTALL $(docdata) makefile makefile.dj makefile.bcc \
	main.cpp auto_dir.h $(SRC_CPP:=.h) $(SRC_C:=.h) \
	$(SRC_CPP:=.cpp) $(SRC_C:=.c) id3.man \
	$(DIR_DEBIAN:%=debian/%)

D_VER = `sed -n "/_version_/{s:[^0-9]*\([^ ]*\).*:\1:p;q;}" main.cpp`

D_PKG = pkg=id3-$(D_VER); \
	rm -f $${pkg}; \
	ln -s `pwd` $${pkg}

D_FIL = `echo $(DISTFILES) | sed "s:[^ ]*:$${pkg}/&:g"`

D_TMP = rm -rf .tmp; mkdir .tmp && \
	tar cf - $(DISTFILES) | tar Cxf .tmp -

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
	$(D_TMP) && make -C .tmp all && mv .tmp/*.o .tmp/id3 .tmp/id3l `pwd`
	-rm -rf .tmp
	@echo all release checks okay

dist-clean: $(DISTFILES)
	$(D_TMP)
	-rm -rf *
	mv .tmp/* `pwd`
	-rm -rf .tmp

orig ?= `pwd`.tar.gz

diff:
	rm -rf .tmp; mkdir .tmp && ln -s `pwd` .tmp/{current}
	tar Cxfz .tmp $(orig)
	diff -x '.*' -durN .tmp/* | gzip -9 > `pwd`-$(D_VER).diff.gz
	-rm -rf .tmp

URI	 = http://home.wanadoo.nl/squell
HTMLPROC = sed -n 's/^.*href="\([[:alnum:]/_.-]*[.]tar[.]gz\)".*$$/\1/p'

wget-orig:
	which wget
	wget -nv $(URI)/`wget -nv -O - $(URI)/id3.html | $(HTMLPROC)`

fetch-orig:
	which fetch
	fetch $(URI)/`fetch -o - $(URI)/id3.html | $(HTMLPROC)`

curl-orig:
	which curl
	curl -# -O $(URI)/`curl -# $(URI)/id3.html | $(HTMLPROC)`

## build rules #############################################################

OBJ_GEN = sedit varexp ffindexp charconv
OBJ_1	= setid3 id3v1
OBJ_2	= setid3v2 id3v2 fileops
OBJ_F	= setfname
OBJECTS = main $(OBJ_GEN) set_base $(OBJ_1) $(OBJ_2) $(OBJ_F)
OBJX_L	= mainl $(OBJ_GEN) set_base $(OBJ_1) $(OBJ_F)

id3: $(OBJECTS:=.o)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS:=.o)

id3l: $(OBJX_L:=.o)
	$(CXX) $(LDFLAGS) -o $@ $(OBJX_L:=.o)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CFLAGS) -c $<

mainl.o:
	$(CXX) $(CXXFLAGS) -DNO_V2 -o $@ -c main.cpp

MKALLDEP =  $(MKDEP) $(CXXFLAGS) main.cpp;
MKALLDEP += $(MKDEP) $(CXXFLAGS) -DNO_V2 main.cpp | sed 's/main.o/mainl.o/';
MKALLDEP += $(MKDEP) $(CXXFLAGS) $(SRC_CPP:=.cpp);
MKALLDEP += $(MKDEP) $(CFLAGS)	 $(SRC_C:=.c)

## dependencies -MM ########################################################

main.o: main.cpp ffindexp.h auto_dir.h sedit.h set_base.h setid3.h \
  setfname.h setid3v2.h
mainl.o: main.cpp ffindexp.h auto_dir.h sedit.h set_base.h setid3.h \
  setfname.h
sedit.o: sedit.cpp charconv.h sedit.h
varexp.o: varexp.cpp varexp.h
ffindexp.o: ffindexp.cpp varexp.h auto_dir.h ffindexp.h
charconv.o: charconv.cpp charconv.h
set_base.o: set_base.cpp set_base.h sedit.h
setid3.o: setid3.cpp setid3.h set_base.h sedit.h id3v1.h
setid3v2.o: setid3v2.cpp setid3v2.h set_base.h sedit.h id3v1.h id3v2.h \
  fileops.h
setfname.o: setfname.cpp setfname.h set_base.h sedit.h
fileops.o: fileops.c fileops.h
id3v1.o: id3v1.c id3v1.h
id3v2.o: id3v2.c fileops.h id3v2.h

############################################################################
