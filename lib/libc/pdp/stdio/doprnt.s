/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifdef LIBC_SCCS
/*	<@(#)doprnt.s	5.4 (Berkeley) 1/25/87\0> */
	<@(#)doprnt.s	5.6 (2.11BSD GTE) 2/02/95\0>
	.even
#endif LIBC_SCCS

/*
 * doprnt(fmt, argl, iop)
 *	char	*fmt, *argl;
 *	FILE	*iop;
 *
 * Output argl to iop using fmt - implements printf, etc.
 */
#include "DEFS.h"

	.data
swtab:
	decimal;	'd
	octal;		'o
	hex;		'x
	float;		'f
	scien;		'e
	general;	'g
	charac;		'c
	string;		's
	longorunsg;	'l
	longorunsg;	'L
	unsigned;	'u
	long;		'D
	loct;		'O
	lhex;		'X
	lunsigned;	'U
	0;  0
.text

.globl	__strout, csv, cret
.globl	pgen, pfloat, pscien

width	= -10.
formp	= -12.
rjust	= -14.
ndfnd	= -16.
ndigit	= -18.
zfill	= -20.

ENTRY(_doprnt)
	jsr	r5,csv
	sub	$300.+12.,sp		/ was 128.
	mov	4(r5),formp(r5)		/ format
	mov	6(r5),r4
loop:
	mov	sp,r3
	mov	formp(r5),r1
2:
	movb	(r1)+,r2
	beq	2f
	cmp	r2,$'%
	beq	2f
	movb	r2,(r3)+
	br	2b
2:
	mov	r1,formp(r5)
	cmp	r3,sp
	beq	2f
	mov	sp,r0
	mov	8(r5),-(sp)
	clr	-(sp)
	mov	r0,-(sp)	/ string
	mov	r3,-(sp)	/ count
	sub	r0,(sp)
	jsr	pc,__strout
	add	$8,sp
2:
	tst	r2
	bne	2f
	jmp	cret
2:
	mov	sp,r3
2:
	clr	rjust(r5)
	clr	ndigit(r5)
	mov	$' ,zfill(r5)
	cmpb	*formp(r5),$'-
	bne	2f
	inc	formp(r5)
	inc	rjust(r5)
2:
	cmpb	*formp(r5),$'0
	bne	2f
	mov	$'0,zfill(r5)
2:
	jsr	r3,gnum
	mov	r1,width(r5)
	clr	ndfnd(r5)
	cmp	r0,$'.
	bne	1f
	jsr	r3,gnum
	mov	r1,ndigit(r5)
1:
	mov	$swtab,r1
1:
	mov	(r1)+,r2
	bne	2f
	movb	r0,(r3)+
	jmp	prbuf
2:
	cmp	r0,(r1)+
	bne	1b
	jmp	(r2)

general:
	mov	ndigit(r5),r0
	mov	ndfnd(r5),r2
	jsr	pc,pgen
	jbr	prbuf

longorunsg:
	movb	*formp(r5),r0
	inc	formp(r5)
	cmp	r0,$'o
	beq	loct
	cmp	r0,$'x
	beq	lhex
	cmp	r0,$'d
	beq	long
	cmp	r0,$'u
	beq	lunsigned
	dec	formp(r5)
	br	unsigned

octal:
	clr	r0
	br	1f
loct:
	mov	(r4)+,r0
1:
	mov	$8.,r2
	br	2f

hex:
	clr	r0
	br	1f

lhex:
	mov	(r4)+,r0
1:
	mov	$16.,r2
2:
	mov	(r4)+,r1
	br	compute

decimal:
	mov	(r4)+,r1
	sxt	r0
	bmi	3f
	br	2f

unsigned:
	clr	r0
	br	1f

long:
	mov	(r4)+,r0
	bge	1f
	mov	(r4)+,r1
3:
	neg	r0
	neg	r1
	sbc	r0
	movb	$'-,(r3)+
	br	2f

lunsigned:
	mov	(r4)+,r0
1:
	mov	(r4)+,r1
2:
	mov	$10.,r2

/*
 * Algorithm courtesy Keith Davis
 */
compute:
	mov	r5,-(sp)
	mov	r4,-(sp)
	mov	r0,r4
	mov	ndigit(r5),r0
	mov	r1,r5
	ashc	$0,r4
	beq	1f
	tst	r0
	beq	1f
	movb	$'0,(r3)+
1:
	jsr	pc,1f
	mov	(sp)+,r4
	mov	(sp)+,r5
	br	prbuf

1:
	clr	r0
	mov	r4,r1
	beq	2f
	div	r2,r0
	mov	r0,r4
	mov	r1,r0
2:
	mov	r5,r1
	asl	r2
	div	r2,r0
	asr	r2
	asl	r0
	cmp	r2,r1
	bgt	2f
	sub	r2,r1
	inc	r0
2:
	mov	r1,-(sp)
	mov	r0,r5
	bne	2f
	tst	r4
	beq	1f
2:
	jsr	pc,1b
1:
	mov	(sp)+,r0
	add	$'0,r0
	cmp	r0,$'9
	ble	1f
	add	$'a-'0-10.,r0
1:
	movb	r0,(r3)+
	rts	pc
	
charac:
	mov	$' ,zfill(r5)
	mov	(r4)+,r0
	movb	r0,(r3)+
	br	prbuf

string:
	mov	$' ,zfill(r5)
	mov	ndigit(r5),r1
	mov	(r4),r2
	mov	r2,r3
1:
	tstb	(r2)+
	beq	1f
	inc	r3
	sob	r1,1b
1:
	mov	(r4)+,r2
	br	prstr

float:
	mov	ndigit(r5),r0
	mov	ndfnd(r5),r2
	jsr	pc,pfloat
	br	prbuf

scien:
	mov	ndigit(r5),r0
	inc	r0
	tst	ndfnd(r5)
	bne	1f
	mov	$7,r0
1:
	mov	pc,r2
	jsr	pc,pscien

prbuf:
	mov	sp,r2
prstr:
	sub	r2,r3
	mov	width(r5),r1
	sub	r3,r1
	bge	1f
	clr	r1
1:
	tst	rjust(r5)
	bne	1f
	neg	r1
1:
	mov	zfill(r5),-(sp)
	mov	8(r5),-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)	/ string
	mov	r3,-(sp)	/ count
	jsr	pc,__strout
	add	$10.,sp
	jmp	loop

gnum:
	clr	ndfnd(r5)
	clr	r1
1:
	movb	*formp(r5),r0
	inc	formp(r5)
	sub	$'0,r0
	cmp	r0,$'*-'0
	bne	2f
	mov	(r4)+,r0
	br	3f
2:
	cmp	r0,$9.
	bhi	1f
3:
	inc	ndfnd(r5)
	mul	$10.,r1
	add	r0,r1
	br	1b
1:
	add	$'0,r0
	rts	r3
