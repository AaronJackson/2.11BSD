#
# @(#)Makefile  1.0 (2.11BSD) 2020/3/4
#

CFLAGS= -O
SEPFLAG= -i
MAN=	top.0
MANSRC=	top.1

all: top ${MAN}

top: top.o psdb.o
	cc -i -o top top.o psdb.o -lcurses -ltermcap

top.o: top.c psdb.h
	cc ${CFLAGS} -c top.c

psdb.o: psdb.c psdb.h
	cc ${CFLAGS} -c psdb.c

top.0: top.1
	/usr/man/manroff ${MANSRC} > ${MAN}

install: all
	install -s -o root -g kmem -m 2755 top ${DESTDIR}/usr/ucb
	install -c -o bin -g bin -m 444 ${MAN} ${DESTDIR}/usr/man/cat1

clean:
	rm -f top.o psdb.o top ${MAN}
