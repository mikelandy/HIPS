#
#	 - makefile for libMplVipLib (Matrox VIP-1024 board routines).
#

CC = /bin/cc
DESTDIR=/usr
CFLAGS = -O $(INCLUDES)
OBJS = setup.o graphics.o videoIO.o

all:	${OBJS}

.c.o:
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

clean:
	rm -f ${OBJS}

libMplVip.a:	all
	ar cr libMplVip.a `lorder *.o | tsort`;
	rm -f t1

install:libMplVip.a
	install libMplVip.a $(DESTDIR)/lib/libMplVip.a
	ranlib $(DESTDIR)/lib/libMplVip.a
