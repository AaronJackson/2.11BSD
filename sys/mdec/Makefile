#
# Copyright (c) 1980, 1986 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	2.0 (2.11BSD) 2022/8/30
#
ALL=	bruboot hkuboot rauboot rkuboot rluboot si95uboot rx02uboot \
	rx01uboot

MBDISK= hpuboot rm03uboot rm05uboot rp07uboot dvhpuboot si51uboot si94uboot

all: ${ALL} ${MBDISK}

${ALL}:
	/lib/cpp -t -E $*.s | sed -e 's;^#;/;' | as -o a.out
	nm -u a.out
	size a.out
	strip a.out
	dd if=a.out of=b.out bs=16 skip=1
	dd if=b.out of=$@ conv=sync

${MBDISK}: mbuboot.s
	/lib/cpp -t -D$@ -E mbuboot.s | sed -e 's;^#;/;' | as -o a.out
	nm -u a.out
	size a.out
	strip a.out
	dd if=a.out of=b.out bs=16 skip=1
	dd if=b.out of=$@ conv=sync

bruboot:	bruboot.s
hkuboot:	hkuboot.s
rauboot:	rauboot.s
rkuboot:	rkuboot.s
rluboot:	rluboot.s
si95uboot:	si95uboot.s
rx02uboot:	rx02uboot.s
rx01uboot:	rx01uboot.s

clean:
	rm -f x.s a.out b.out ${ALL} ${MBDISK}

install: all
	cp *boot ${DESTDIR}/mdec
