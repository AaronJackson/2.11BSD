# @(#)Makefile	1.8 Makefile (2.11BSD) 2019/11/21

DESTDIR=

# If you want code inspired by certain emerging standards, add
#	-DSTD_INSPIRED
# to the end of the "CFLAGS=" line.
#
# If you want to handle solar-time-based time zones, remove the
# "#define NOSOLAR" from the include file usr/include/tzfile.h.
# (and add solar87 to the DATA= line below).

CFLAGS= -O
SEPFLAG=-i
LINTFLAGS=	-phbaxc
CC=		cc

TZCSRCS=	zic.c scheck.c ialloc.c
TZCOBJS=	zic.o scheck.o ialloc.o
TZDSRCS=	zdump.c ialloc.c
TZDOBJS=	zdump.o ialloc.o
SOURCES=	zic.c zdump.c scheck.c ialloc.c
DATA=		asia australasia europe etcetera northamerica \
		pacificnew systemv
ZICMAN=		zic.0
ZICMANSRC=	zic.8
ZDUMAN=		zdump.0
ZDUMANSRC=	zdump.8
MAN=		$(ZICMAN) $(ZDUMAN)
MANSRC=		$(ZICMANSRC) $(ZDUMAN)
SHARDIR=	${DESTDIR}/usr/share/zoneinfo
MANDIR=		${DESTDIR}/usr/man/cat8

all:		zdump zic ${ZICMAN} ${ZDUMAN}

install:	zic $(DATA) $(MAN)
		(umask 22; ./zic -d ${SHARDIR} $(DATA))
		install -c -m 444 -o bin -g bin $(ZICMAN) ${MANDIR}/${ZICMAN}
		install -c -m 444 -o bin -g bin $(ZDUMAN) ${MANDIR}/${ZDUMAN}
		install -c -s -m 755 -o bin -g bin zic ${DESTDIR}/usr/sbin/zic
		install -c -s -m 755 -o bin -g bin zdump ${DESTDIR}/usr/sbin/zdump

zdump.0:	$(ZDUMANSRC)
		/usr/man/manroff $(ZDUMANSRC) > $(ZDUMAN)

zic.0:		$(ZICMANSRC)
		/usr/man/manroff $(ZICMANSRC) > $(ZICMAN)

zdump:		$(TZDOBJS)
		$(CC) $(CFLAGS) $(LFLAGS) $(SEPFLAG) $(TZDOBJS) -o $@

zic:		$(TZCOBJS)
		$(CC) $(CFLAGS) $(LFLAGS) $(SEPFLAG) $(TZCOBJS) -o $@

lint:		$(TZCSRCS) $(TZDSRCS)
		lint $(LINTFLAGS) $(CFLAGS) $(TZCSRCS)
		lint $(LINTFLAGS) $(CFLAGS) $(TZDSRCS)

clean:
		rm -f *.o zdump zic $(ZICMAN) $(ZDUMAN)
