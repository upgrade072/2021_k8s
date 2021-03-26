include ../MakeVars

CFLAG			= $(CFLAGS) -D_GNU_SOURCE -Wall -std=gnu99

BUILD_INC_PATH	= -I../build/include -I../../../include
BUILD_LIBS_PATH	= ../build/lib
BUILD_LIBS = $(BUILD_LIBS_PATH)/libssl.a \
			 $(BUILD_LIBS_PATH)/libcrypto.a \
			 $(BUILD_LIBS_PATH)/libevent_openssl.a \
             $(BUILD_LIBS_PATH)/libevent_pthreads.a \
             $(BUILD_LIBS_PATH)/libevent.a \
             $(BUILD_LIBS_PATH)/libfort.a \
             $(BUILD_LIBS_PATH)/libtopo.a \
             $(BUILD_LIBS_PATH)/libcomm.a \
             $(BUILD_LIBS_PATH)/libs.a \
             -lpthread -lrt -lm -ldl

SRCS        = main.c

OBJS        = $(SRCS:.c=.o)

AOUT        = k8smgr

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
