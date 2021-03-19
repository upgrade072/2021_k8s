include ../MakeVars

AOUT        = libfort.a
DEST        = ../build

#
#---------------------------------------------------------------------
#
all:: clean $(AOUT) install

.SUFFIXES: .c .o
.c.o:

$(AOUT): $(OBJS)

install:
	\cp *.h $(DEST)/include
	\cp *.a $(DEST)/lib

clean:
