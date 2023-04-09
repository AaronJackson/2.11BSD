#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	4.8	(2.11BSD) 2020/3/24
#
# Possible options:
#	make		compile source for all terminal tables below
#	make <term>	compile table for a specific <term>
#	make install	move tables to ${DEST}
#	make clean	remove tab*.o files lying around
CFLAGS=	-I../common_source
DEST=	${DESTDIR}/usr/share/term
SRCS=	tab300-12.c tab300.c tab302-12.c tab302.c tab37.c tab382-12.c \
	tab382.c tab450-12.c tab450.c tab833-12.c tab833.c tabepson.c \
	tabitoh.c tabitoh12.c tablpr.c tabnec-t.c tabnec.c tabnec12.c \
	tabnec25-t.c tabqume.c tabqume12.c tabx-ecs.c tabx-ecs12.c \
	tabxerox.c tabxerox12.c
OBJS=	tab300-12.o tab300.o tab302-12.o tab302.o tab37.o tab382-12.o \
	tab382.o tab450-12.o tab450.o tab833-12.o tab833.o tabepson.o \
	tabitoh.o tabitoh12.o tablpr.o tabnec-t.o tabnec.o tabnec12.o \
	tabnec25-t.o tabqume.o tabqume12.o tabx-ecs.o tabx-ecs12.o \
	tabxerox.o tabxerox12.o
ALL=	tab37.tab tablpr.tab tab300.tab tab300-12.tab tab302.tab \
	tab302-12.tab tab382.tab tab382-12.tab tab450.tab tab450-12.tab \
	tab833.tab tab833-12.tab tabepson.tab tabitoh.tab tabitoh12.tab \
	tabnec.tab tabnec12.tab tabnec-t.tab tabnec25-t.tab tabqume.tab \
	tabqume12.tab tabxerox.tab tabxerox12.tab tabx-ecs.tab tabx-ecs12.tab

.SUFFIXES: .tab

all: ${ALL}

${OBJS}: mktab.o

.o.tab:
	cc ${CFLAGS} mktab.o $< -o $@.tmp && ./$@.tmp > $@

37:		tab37.tab
lpr:		tablpr.tab
300:		tab300.tab
300-12:		tab300-12.tab
302:		tab302.tab
302-12:		tab302-12.tab
382:		tab382.tab
382-12:		tab382-12.tab
450:		tab450.tab
450-12:		tab450-12.tab
833:		tab833.tab
833-12:		tab833-12.tab
epson:		tabepson.tab
itoh:		tabitoh.tab
itoh12:		tabitoh12.tab
nec:		tabnec.tab
nec12:		tabnec12.tab
nec-t:		tabnec-t.tab
nec25-t:	tabnec25-t.tab
qume:		tabqume.tab
qume12:		tabqume12.tab
xerox:		tabxerox.tab
xerox12:	tabxerox12.tab
x-ecs:		tabx-ecs.tab
x-ecs12:	tabx-ecs12.tab

clean: FRC
	rm -f ${OBJS} core ${ALL} mktab.o *.tmp

depend: FRC
	mkdep ${CFLAGS} ${SRCS}

install: ${ALL}
	-mkdir -p ${DEST}
	chmod 755 ${DEST}
	for file in tab*.tab; do \
		install -c -o bin -g bin -m 755 $$file ${DEST}/`basename $$file`; \
	done
	rm -f ${DEST}/tabtn300.tab; ln ${DEST}/tablpr.tab ${DEST}/tabtn300.tab
	rm -f ${DEST}/tabcrt.tab; ln ${DEST}/tablpr.tab ${DEST}/tabcrt.tab
	rm -f ${DEST}/tab300s.tab; ln ${DEST}/tab302.tab ${DEST}/tab300s.tab
	rm -f ${DEST}/tab300s-12.tab; ln ${DEST}/tab302-12.tab ${DEST}/tab300s-12.tab
	rm -f ${DEST}/tabdtc.tab; ln ${DEST}/tab302.tab ${DEST}/tabdtc.tab
	rm -f ${DEST}/tabdtc12.tab; ln ${DEST}/tab302-12.tab ${DEST}/tabdtc12.tab
	rm -f ${DEST}/tabipsi.tab; ln ${DEST}/tab450.tab ${DEST}/tabipsi.tab
	rm -f ${DEST}/tabipsi12.tab; ln ${DEST}/tab450-12.tab ${DEST}/tabipsi12.tab
	install -c -o bin -g bin -m 644 README ${DEST}/README

lint: FRC
	lint ${CFLAGS} ${SRCS}

tags: FRC
	ctags ${SRCS}

FRC:

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

