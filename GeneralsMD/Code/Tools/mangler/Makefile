all = mangler

TARGET=mangler
TESTTARGET=manglertest

OBJECTS=mangler.o crc.o
TESTOBJECTS=manglertest.o crc.o


LINKFLAGS=
LINKDIRS=-L. -Lwlib -Lwnet
############################################################################
### -lpthread ### MUST MUST MUST be the last library!
############################################################################
LINKLIBS = -lwnet -lwlib -lsocket -lnsl -lposix4 -lpthread

INCDIR=-I. -Iwlib -Iwnet

CPPFLAGS=$(INCDIR) -c -gstabs+ -DDEBUG -D_UNIX
#CPPFLAGS=$(INCDIR) -c -gstabs+ -D_UNIX

CPPFILES=${OBJECTS:.o=.cpp}
CFILES=${OBJECTS:.o=.c}
.SUFFIXES : .c .cpp


$(TARGET) : $(OBJECTS)
	g++ $(LINKFLAGS) $(OBJECTS) $(LINKDIRS) $(LINKLIBS) -o $(TARGET)

$(TESTTARGET) : $(TESTOBJECTS) $(TARGET)
	g++ $(LINKFLAGS) $(TESTOBJECTS) $(LINKDIRS) $(LINKLIBS) -o $(TESTTARGET)

.cpp.o:
	g++ $(CPPFLAGS) $<

.c.o:
	g++ $(CPPFLAGS) $<


depend:
	g++ -M $(INCDIR) ${CFILES} $(CPPFILES) > dependencies


clean:
	-rm *.o $(TARGET) $(TESTTARGET) core *.dat

rebuild:
	make clean; make

run:
	make; $(TARGET)

debug:
	gdb -s noxc noxc

dist:
	make clean; make; cp wdtd gavin; cp START gavin; cp STOP gavin; cp world.wdt gavin;

#include dependencies
