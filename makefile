## generic GNU makefile ####################################################

SHELL	 = /bin/sh

CC	 = cc
CXX	 = c++
CFLAGS	 = -Wformat -g -Os
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
.PHONY: dist dist-zip dist-clean dist-check
.PHONY: git-repo

.SUFFIXES: .c .cpp .o

## standard targets ########################################################

build: id3

all  : id3 id3l id3-images

final: id3 id3l id3-images
	$(STRIP) id3 id3l id3-images

clean:
	-rm -f *.o id3 id3l id3-images

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
	$(INSTALL_PROGRAM) $(binary) $(bindir)/id3

install-strip: $(binary) installdirs installman
	$(INSTALL_STRIP) $(binary) $(bindir)/id3

install-full: install-strip installdoc

uninstall:
	-rm -f $(man1dir)/id3.$(manext)
	-rm -f $(bindir)/id3
	-for f in $(docdata); do \
	    rm -f $(docdir)/$${f}; done
	-rmdir $(docdir)

## distribution ############################################################

SRC_CPP     = sedit varexp fileexp mass_tag pattern	\
	      charconv char_ucs char_utf8 lyrics3	\
	      setid3 setid3v2 setlyr3 setfname setquery \
	      getid3 getid3v2 getlyr3
SRC_C	    = fileops id3v1 id3v2
DIR_DEBIAN  = control rules copyright changelog
DIR_RPM     = id3mtag.spec
DIR_FREEBSD = Makefile pkg-descr

DISTFILES = INSTALL $(docdata) makefile makefile.dj makefile.bcc \
	main.cpp id3-images.c auto_dir.h set_base.h setgroup.h $(SRC_CPP:=.h) $(SRC_C:=.h) \
	$(SRC_CPP:=.cpp) $(SRC_C:=.c) id3.man \
	$(DIR_DEBIAN:%=debian/%) \
	$(DIR_RPM:%=rpm/%) \
	$(DIR_FREEBSD:%=FreeBSD/%)

D_VER = `sed -n "/_version_/{s:[^0-9]*\([^ ]*\).*:\1:p;q;}" main.cpp`

D_PKG = pkg=id3-$(D_VER); \
	rm -f $${pkg}; \
	ln -s `pwd` $${pkg}

D_FIL = `echo $(DISTFILES) | sed "s:[^ ]*:$${pkg}/&:g"`

D_TMP = rm -rf .tmp; mkdir .tmp && \
	$(TAR) cf - $(DISTFILES) | $(TAR) Cxf .tmp -

dist: $(DISTFILES)
	$(D_PKG) && $(TAR) chofz $${pkg}.tar.gz $(D_FIL); rm -f $${pkg}

dist-zip: $(DISTFILES)
	$(D_PKG) && zip -9Tl $${pkg//.}s.zip $(D_FIL); rm -f $${pkg}

dist-check: all $(DISTFILES)
	if [ -f !* ]; then echo !*; false; fi
	test `sed -n '/^#define _version_.*(\(.*\)).*/s//\1/p' < main.cpp` \
	   = `date +%Y%j`
	grep $(D_VER) INSTALL
	grep $(D_VER) CHANGES
	d=`(cat INSTALL | sed -n -e '/contents/,/---/!d' \
	  -e '/(C++)$$/{ s/^ \([^ ]*\)[*].*/\1h/p; s/h$$/cpp/p;}' \
	  -e '/(C)$$/  { s/^ \([^ ]*\)[*].*/\1h/p; s/h$$/c/p;  }' \
	  -e		's/^ \([^ /]*\).*/\1/p'; \
	    ls $(DISTFILES) | sed 's:/[^ ]*::g') | sort | uniq -u` && \
	echo "$${d}"; test -z "$${d}"
	test -x debian/rules
	@echo all release checks okay

dist-clean: $(DISTFILES)
	$(D_TMP)
	-rm -rf *
	mv .tmp/* `pwd`
	-rm -rf .tmp

git-repo: .git
.git:	# transform into a git working tree
	which git
	git init
	git remote add origin -t master -f git@github.com:squell/id3.git
	git reset FETCH_HEAD

AWKCMD = "BEGIN { pretty = \"figlet -fmini | sed '\$$s/ /~/g'\" } \
	/ID3\(1\)/ { next } \
	 /^[A-Z]/ { print tolower(\$$0) | pretty; close(pretty) } \
	!/^[A-Z]/ { print }"

README: id3.man
	@man -l id3.man | col -b | awk $(AWKCMD) | diff -u README -

## build rules #############################################################

OBJ_GEN = sedit varexp fileexp charconv mass_tag pattern
OBJ_1	= setid3 getid3 id3v1
OBJ_2	= setid3v2 getid3v2 id3v2 fileops char_ucs char_utf8
OBJ_3	= setlyr3 getlyr3 lyrics3
OBJ_F	= setfname setquery
OBJECTS = main $(OBJ_GEN) $(OBJ_1) $(OBJ_2) $(OBJ_3) $(OBJ_F)
OBJX_L	= mainl $(OBJ_GEN) $(OBJ_1) $(OBJ_F)

id3: $(OBJECTS:=.o)
	$(CXX) $(CXXFLAGS) $(OBJECTS:=.o) $(LDFLAGS) -o $@

id3l: $(OBJX_L:=.o)
	$(CXX) $(CXXFLAGS) $(OBJX_L:=.o) $(LDFLAGS) -o $@

id3-images: id3-images.c id3v2.c id3v2.h
	$(CC) $(CFLAGS) $(LDFLAGS) id3-images.c id3v2.c -DID3v2_READONLY -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CFLAGS) -c $<

mainl.o:
	$(CXX) $(CXXFLAGS) -DLITE -o $@ -c main.cpp

MKALLDEP = $(MKDEP) $(CXXFLAGS) main.cpp;				   \
	   $(MKDEP) $(CXXFLAGS) -DLITE main.cpp | sed 's/main.o/mainl.o/'; \
	   $(MKDEP) $(CXXFLAGS) $(SRC_CPP:=.cpp);			   \
	   $(MKDEP) $(CFLAGS)	$(SRC_C:=.c)

## dependencies -MM ########################################################

main.o: main.cpp setgroup.h set_base.h sedit.h charconv.h setid3.h \
  setfname.h setquery.h setid3v2.h setlyr3.h mass_tag.h fileexp.h \
  pattern.h
mainl.o: main.cpp setgroup.h set_base.h sedit.h charconv.h setid3.h \
  setfname.h setquery.h mass_tag.h fileexp.h pattern.h
sedit.o: sedit.cpp sedit.h charconv.h
varexp.o: varexp.cpp varexp.h
fileexp.o: fileexp.cpp varexp.h auto_dir.h fileexp.h
mass_tag.o: mass_tag.cpp charconv.h sedit.h set_base.h mass_tag.h \
  fileexp.h
pattern.o: pattern.cpp set_base.h sedit.h charconv.h mass_tag.h fileexp.h \
  pattern.h
charconv.o: charconv.cpp charconv.h
char_ucs.o: char_ucs.cpp char_ucs.h charconv.h
char_utf8.o: char_utf8.cpp utf8.h char_utf8.h charconv.h
lyrics3.o: lyrics3.cpp lyrics3.h
setid3.o: setid3.cpp id3v1.h getid3.h set_base.h sedit.h charconv.h \
  setid3.h
setid3v2.o: setid3v2.cpp char_ucs.h charconv.h id3v1.h id3v2.h fileops.h \
  getid3v2.h set_base.h sedit.h setid3v2.h
setlyr3.o: setlyr3.cpp charconv.h lyrics3.h getlyr3.h getid3.h set_base.h \
  sedit.h id3v1.h setlyr3.h
setfname.o: setfname.cpp sedit.h charconv.h setfname.h setgroup.h \
  set_base.h
setquery.o: setquery.cpp setquery.h set_base.h sedit.h charconv.h
getid3.o: getid3.cpp getid3.h set_base.h sedit.h charconv.h id3v1.h
getid3v2.o: getid3v2.cpp char_ucs.h charconv.h char_utf8.h id3v2.h \
  getid3v2.h set_base.h sedit.h
getlyr3.o: getlyr3.cpp lyrics3.h getid3.h set_base.h sedit.h charconv.h \
  id3v1.h getlyr3.h
fileops.o: fileops.c fileops.h
id3v1.o: id3v1.c id3v1.h
id3v2.o: id3v2.c fileops.h id3v2.h

############################################################################
