include ../MakeVars

CFLAG			= $(CFLAGS) -D_GNU_SOURCE -Wall -std=gnu99 -pg

BUILD_INC_PATH	= -I../build/include -I../../../include
BUILD_LIBS_PATH	= ../build/lib
BUILD_LIBS = \
             $(BUILD_LIBS_PATH)/libcomm.a \
             -lpthread -lrt -lm

SRCS        = main.c

OBJS        = $(SRCS:.c=.o)

AOUT        = proc_b

DEST        = ../bin

#
#---------------------------------------------------------------------
#
all:: clean $(AOUT) install

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(BUILD_INC_PATH) $(INC_PATH) -c $<

$(AOUT): $(OBJS)
	$(CC) $(CFLAG) -o $(AOUT) $(OBJS) $(BUILD_LIBS) $(LIB_PATH) $(LIBS) 

install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core.* 
	rm -rf cscope.out tceetree.out tceetree.out.png
	rm -rf *.csv
