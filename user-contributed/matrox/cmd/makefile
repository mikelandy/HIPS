DESTDIR=/usr/hips
CFLAGS=	-O
LIB= -lhips
MATROXLIB= -lMplVip
#OBJS=	wframe.o mclear.o rframe.o mlive.o msnap.o
OBJS=	rframe.o wframe.o

all:${OBJS} 

.c.o:
	${CC} ${CFLAGS} -o $* $*.c ${LIB} ${MATROXLIB}
	install -s $* ${DESTDIR}

clean:
	rm -f ${OBJS}
