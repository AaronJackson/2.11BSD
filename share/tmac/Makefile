#
# Copyright (c) 1985 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	6.3 (2.11BSD) 2020/3/24
#
#
#  Makefile for -man macros
#

SRCS=	tmac.a tmac.an tmac.an.new tmac.an6n tmac.an6t tmac.ayday \
	tmac.cp tmac.imagen tmac.os tmac.r tmac.s tmac.scover \
	tmac.sdisp tmac.skeep tmac.srefs tmac.vcat
MSRCS=	doc doc-ditroff doc-common doc-nroff doc-syms

all:	${SRCS}

install:${SRCS}
	-mkdir -p ${DESTDIR}/usr/share/tmac
	-chmod a+r,a+x ${DESTDIR}/usr/share/tmac
	install -c -m 444 ${SRCS} ${DESTDIR}/usr/share/tmac
	for i in ${MSRCS}; do \
		sed -f strip.sed < $$i > ${DESTDIR}/usr/share/tmac/tmac.$$i; \
		chmod 444 ${DESTDIR}/usr/share/tmac/tmac.$$i; \
	done

clean:
