## generic GNU makefile ####################################################

SHELL	 = /bin/sh

CC	 = gcc
CFLAGS	 = -g -O2

CXX	 = g++

############################################################################

id3: main.o varexp.o setid3.o setid3v2.o id3v1.o id3v2.o
	$(CXX) -o $@ $+

id3l: mainl.o varexp.o setid3.o id3v1.o
	$(CXX) -o $@ $+

all  : id3 id3l

final: id3 id3l
	strip $+

clean:
	rm *.o id3 id3l

############################################################################

main.o: main.cpp varexp.h setid3.h setid3v2.h id3v1.h id3v2.h
	$(CC) $(CFLAGS) -c main.cpp

mainl.o: main.cpp varexp.h setid3.h id3v1.h
	$(CC) $(CFLAGS) -DNO_V2 -o $@ -c main.cpp

setid3v2.o: id3v2.h setid3.h
setid3.o  : id3v1.h

%.o : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $<

############################################################################

