#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley Software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4 (2.11BSD) 2020/1/7
#
# C Shell with process control; VM/UNIX VAX Makefile
# Bill Joy UC Berkeley; Jim Kulp IIASA, Austria
#
# To profile, put -DPROF in DEFS and -pg in CFLAGS, and recompile.

DEFS=	-DTELL -DVFORK -DFILEC
CFLAGS=	$(DEFS) -O
SEPFLAG=-i
XSTR=	/usr/ucb/xstr
AS=	as
RM=	-rm
CXREF=	/usr/ucb/cxref
VGRIND=	/usr/ucb/vgrind
CTAGS=	/usr/ucb/ctags
LIBES=  -lc
SCCS=	sccs
CPP=	/lib/cpp -t
SED=	sed

BASE=	alloc11.o doprnt11.o printf.o sh.char.o sh.dir.o sh.dol.o \
	sh.err.o sh.exp.o sh.file.o sh.func.o sh.glob.o sh.hist.o \
	sh.init.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.proc.o sh.sem.o \
	sh.set.o
OV1= 	sh.o sh.exec.o sh.time.o
OV2=	sh.exec2.o

OBJS=	${BASE} ${OV1} ${OV2}

LIBOBJS= ndbm.o getpwent.o

# Special massaging of C files for sharing of strings
.c.o:
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
	${CC} -c ${CFLAGS} x.c 
	mv -f x.o $*.o
	rm -f x.c

# strings.o must be last since it can change when previous files compile
csh: ${OBJS} strings.o
	ar x /lib/libc.a ${LIBOBJS}
	rm -f csh
	/bin/ld ${SEPFLAG} /lib/crt0.o ${BASE} \
		-Z ${OV1} \
		-Z ${OV2} ${LIBOBJS} \
		-Y strings.o -o csh ${LIBES};

.DEFAULT:
	${SCCS} get $<

# need an old doprnt, whose output we can trap
doprnt11.o: doprnt11.s
	${CPP} doprnt11.s | ${SED} '/^#/d' | ${AS} -o doprnt11.o

strings.o: strings
	${XSTR}
	${CC} -c xs.c
	mv -f xs.o strings.o
	rm -f xs.c

lint:
	lint -z ${DEFS} sh*.c alloc.c

print:
	@pr READ_ME
	@pr Makefile Makefile.*
	@(size -l a.out; size *.o) | pr -h SIZES
	@${CXREF} sh*.c | pr -h XREF
	@ls -l | pr 
	@pr sh*.h [a-rt-z]*.h sh*.c alloc.c

vprint:
	@pr -l84 READ_ME TODO
	@pr -l84 Makefile Makefile.*
	@(size -l a.out; size *.o) | pr -l84 -h SIZES
	@${CXREF} sh*.c | pr -l84 -h XREF
	@ls -l | pr -l84
	@${CXREF} sh*.c | pr -l84 -h XREF
	@pr -l84 sh*.h [a-rt-z]*.h sh*.c alloc.c

vgrind:
	@cp /dev/null index
	@-mkdir grind
	for i in *.h; do ${VGRIND} -t -h "C Shell" $$i >grind/$$i.t; done
	for i in *.c; do ${VGRIND} -t -h "C Shell" $$i >grind/$$i.t; done
	${VGRIND} -t -x -h Index index >grind/index.t

install: csh
	install -m 755 -o bin -g bin -s csh ${DESTDIR}/bin/csh

clean:
	${RM} -f a.out strings x.c xs.c csh errs
	${RM} -f *.o
	${RM} -rf vgrind

tags:	*.h sh*.c
	${CTAGS} -t *.h sh*.c

sh.o: sh.h sh.local.h sh.char.h
sh.char.o: sh.char.h
sh.dir.o: sh.h sh.local.h sh.dir.h
sh.dol.o: sh.h sh.local.h sh.char.h
sh.err.o: sh.h sh.local.h sh.char.h
sh.exec.o: sh.h sh.local.h sh.char.h
sh.exec2.o: sh.h sh.local.h sh.char.h
sh.exp.o: sh.h sh.local.h sh.char.h
sh.file.o: sh.h sh.local.h sh.char.h
sh.func.o: sh.h sh.local.h sh.char.h
sh.glob.o: sh.h sh.local.h sh.char.h
sh.hist.o: sh.h sh.local.h sh.char.h
sh.init.o: sh.local.h
sh.lex.o: sh.h sh.local.h sh.char.h
sh.misc.o: sh.h sh.local.h sh.char.h
sh.parse.o: sh.h sh.local.h sh.char.h
sh.print.o: sh.h sh.local.h sh.char.h
sh.proc.o: sh.h sh.local.h sh.dir.h sh.proc.h sh.char.h
sh.sem.o: sh.h sh.local.h sh.proc.h sh.char.h
sh.set.o: sh.h sh.local.h sh.char.h
sh.time.o: sh.h sh.local.h sh.char.h
