## id3 makefile for Borland C++ (tested with 5.5) ##########################

CC     = bcc32
CFLAGS =

LINK   = ilink32
LFLAGS =

############################################################################

id3.exe: main.obj setid3.obj varexp.obj id3v1.obj
        $(CC) $(LFLAGS) -e$@ $**

final: id3.exe
        upx --best $**                          ## requires upx installed ##

all  : id3.exe

clean:
        del *.obj
        del *.exe

.c.obj:
        $(CC) $(CFLAGS) -c $<

.cpp.obj:
        $(CC) $(CFLAGS) -c $<

############################################################################

