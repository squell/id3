CC     = g++
CFLAGS = -g -O2

id3: main.o setid3.o varexp.o
	$(CC) $(CFLAGS) -o $@ $+

final: id3
	strip $<

all  : id3

clean:
	rm *.o
	rm id3

%.o  : %.cpp %.h
	$(CC) $(CFLAGS) -c $<

main.o: main.cpp varexp.h setid3.h
	$(CC) $(CFLAGS) -c $<

