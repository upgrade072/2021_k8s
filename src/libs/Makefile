include ../MakeVars

CFLAG       = $(CFLAGS) -Wno-char-subscripts

SRCS        = elem.c util.c
OBJS        = $(SRCS:.c=.o)

AOUT        = libs.a
DEST        = ../build

#
#---------------------------------------------------------------------
#
all:: clean $(AOUT) install

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(BUILD_INC_PATH) $(INC_PATH) -c $<

$(AOUT): $(OBJS)
	$(AR) -rc $(AOUT) $(OBJS)

install:
	\cp $(AOUT) $(DEST)/lib
	\cp libs.h $(DEST)/include

clean:
	\rm -f $(OBJS) $(AOUT) core.*
