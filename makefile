## generic unix id3 makefile ###############################################

CC     = gcc
CFLAGS = -g -O2

############################################################################

id3: main.o setid3.o varexp.o id3v1.o
	$(CC) $(CFLAGS) -o $@ $+

final: id3
	strip $<

all  : id3

dist :	main.cpp varexp.cpp varexp.h setid3.cpp setid3.h \
    makefile.dj makefile copying
	tar cvfz src.tar.gz $+

clean:
	rm *.o id3

%.o  : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

%.o  : %.c %.h
	$(CC) $(CFLAGS) -c $<

