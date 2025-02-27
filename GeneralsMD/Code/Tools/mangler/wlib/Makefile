###########################################################################
# WLIB library makefile
##########################################################################

#Define cc to be your C compiler
CC = g++ 

#This tells make how to go from a .cpp to a .o
.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} -c $<

INCLUDE = -I. -I..

CFLAGS = ${INCLUDE} -D_REENTRANT -DDEBUG -D_UNIX

AR = ar -r
#CC is dumb and won't include templates in a library uness you define
#CC -xar as your replacement for 'ar'
#AR = CC -xar

RM = rm -f
RANLIB = ranlib

############################################################################
#Dont mess with any of this stuff
OBJECTS = wtime.o monod.o wdebug.o sem4.o streamer.o syslogd.o wstring.o \
            configfile.o threadfac.o critsec.o xtime.o timezone.o

LIBRARY = libwlib.a

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	${RM} libwlib.a
	${AR} libwlib.a $(OBJECTS)
	#${AR} $(OBJECTS) -o libwlib.a
	$(RANLIB) libwlib.a

clean:
	- rm -f $(LIBRARY) *.o core
