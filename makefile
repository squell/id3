## generic GNU makefile ####################################################

CC	 = gcc
CFLAGS	 = -g -O2

CXX	 = g++

############################################################################

id3: main.o setid3.o varexp.o id3v1.o
	$(CXX) -o $@ $+

final: id3
	strip $<

all  : id3

dist :	main.cpp varexp.cpp varexp.h setid3.cpp setid3.h \
    makefile.dj makefile.bcc makefile copying
	tar cvfz src.tar.gz $+

clean:
	rm *.o id3

############################################################################

main.o: main.cpp setid3.h varexp.h id3v1.h
	$(CC) $(CFLAGS) -c $<

%.o  : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

############################################################################

