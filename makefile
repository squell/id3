## generic GNU makefile ####################################################

CC	 = gcc
CFLAGS	 = -g -O2

CXX	 = g++

############################################################################

id3: main.o setid3.o setid3v2.o varexp.o id3v1.o id3v2.o id3_fmt.o
	$(CC) -o $@ $+

id3l: main.o setid3.o varexp.o id3v1.o
	$(CXX) -o $@ $+

all  : id3 id3l

final: id3 id3l
	strip $<

dist :	main.cpp varexp.cpp varexp.h setid3.cpp setid3.h \
    makefile.dj makefile.bcc makefile copying
	tar cvfz src.tar.gz $+

clean:
	rm *.o id3

############################################################################

main.o: main.cpp setid3.h setid3v2.h varexp.h id3v1.h id3v2.h
	$(CC) $(CFLAGS) -c main.cpp

mainl.o: main.cpp setid3.h varexp.h id3v1.h
	$(CC) $(CFLAGS) -DNO_V2 -o $@ -c main.cpp

setid3v2.o: id3v2.h setid3.h
setid3.o  : id3v1.h

%.o  : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

############################################################################

