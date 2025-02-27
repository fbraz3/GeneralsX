###########################################################################
# TCP library makefile
##########################################################################

#Define cc to be your C compiler
CC = g++
CFLAGS = -gstabs ${INCLUDE} -D_REENTRANT #-DDEBUG

#This tells make how to go from a .cpp to a .o
.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} -c $<

INCLUDE = -I.. -I.

AR = ar -r
RM = rm -f
RANLIB = ranlib

############################################################################
#Dont mess with any of this stuff
OBJECTS = tcp.o udp.o packet.o field.o
LIBRARY = libwnet.a

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(RM) $(LIBRARY)
	$(AR) $(LIBRARY) $(OBJECTS)
	$(RANLIB) $(LIBRARY)

clean:
	- rm -f $(LIBRARY) $(OBJECTS) core
