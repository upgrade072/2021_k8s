# -------------------- Makefile which have no SubDir ---------------------
CC			= `which gcc`
AR			= `which ar`
MAKEDEPEND	= `which makedepend`
MAKEFILE	= Makefile

CP			= `which cp`
MV			= `which mv`
RM			= `which rm`

LIBS		= #-lresolv

DEFINES		= 
OPTIMIZE	= -g3 -Wall   #--save-temp

CFLAGS		+= ${OPTIMIZE} ${DEFINES} 
LDFLAGS		= ${LIB_DIRS} ${LIBS}

TEST_LIBS = -ldnsutil
TEST_LIBDIRS = -L./
TEST_LDFLAGS = ${TEST_LIBDIRS} ${TEST_LIBS} 

#-------------------- Source & Objects & Libraries------------------------
TARGET = libdnsutil.a
TARGET_TEST	= dnsQuery


TEST_SRCS = dnsUtilTest.c
SRCS = dnsUtil.c

OBJS = ${SRCS:.c=.o}
TEST_OBJS = ${TEST_SRCS:.c=.o}

#-------------------------------------------------------------------------
.SUFFIXES: .c .o
.c.o:
	${CC} ${CFLAGS} -c $<

all : ${TARGET} install

exam : ${TARGET} ${TARGET_TEST}

${TARGET}: ${OBJS}
	${AR} rcu $@ ${OBJS}

${TARGET_TEST}: ${TEST_OBJS}
	${CC} ${CFLAGS} -o $@ ${TEST_OBJS} ${TEST_LDFLAGS}

clean :
	${RM} -rf ${TARGET} ${OBJS} ${TARGET_TEST} ${TEST_OBJS} core*

install:

