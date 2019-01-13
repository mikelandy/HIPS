DESTDIR=./bin #/horef/image/bin
CFLAGS=	 -O #-I/horef/image/sys
GFLAGS=	 -g #-I/horef/image/sys
LIB= -lhips
ILIB = -lsuntool -lsunwindow -lpixrect
OBJS=   rbuffer.o rframe.o wframe.o convolve.o testconv.o controlfb.o \
	imagetool.o histotool.o
DESTS=rbuffer rframe wframe convolve testconv controlfb imagetool histotool
CSUBS = aluop.o fb_maxmin.o kernel.o
ISUBS = movecursor.o cre_sub.o imtoolproc.o

all:  ${DESTS}

convolve testconv : convolve.h
${CSUBS} : convolve.h
imagetool : imagetool.h
${ISUBS} : imagetool.h

rbuffer: rbuffer.c
	${CC} ${CFLAGS} -o $@ $@.c ${LIB}
	install -s $@ ${DESTDIR}

rframe: rframe.c
	${CC} ${CFLAGS} -o $@ $@.c ${LIB}
	install -s $@  ${DESTDIR}

wframe: wframe.c
	${CC} ${CFLAGS} -o $@ $@.c ${LIB}
	install -s $@  ${DESTDIR}

convolve: convolve.c ${CSUBS}
	${CC} ${CFLAGS} ${CSUBS} -o $@ $@.c ${LIB}
	install -s convolve ${DESTDIR}

testconv: testconv.c ${CSUBS}
	${CC} ${CFLAGS} ${CSUBS} -o $@ $@.c ${LIB}
	install -s testconv ${DESTDIR}

controlfb: controlfb.c
	${CC} ${CFLAGS} -o $@ $@.c ${LIB}
	install -s $@  ${DESTDIR}

imagetool: imagetool.c ${ISUBS} histotool
	${CC} -O ${ISUBS} -o $@ $@.c ${ILIB}
	install -s imagetool ${DESTDIR}

histotool: histotool.c
	${CC} ${CFLAGS} -o $@ $@.c ${ILIB} ${LIB}
	install -s histotool ${DESTDIR}

clean:
	rm -f ${OBJS} ${DESTS} ${ISUBS} ${CSUBS}
