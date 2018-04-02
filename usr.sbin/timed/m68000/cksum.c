/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)cksum.c	2.1 (Berkeley) 6/6/86";
#endif not lint

#include "../globals.h"
#include <protocols/timed.h>

/* computes the checksum for ip packets for a Motorola 68000 base computer */

in_cksum(w, mlen)
	register u_short *w;
	register int mlen;
{
	register int sum = 0;

	if (mlen > 0) {
		if (((int)w & 1) == 0) {
			sum = ocsum(w, mlen>>1);
			w += mlen>>1;
			if (mlen & 1) {
				sum += *(u_char *)w << 8;
				mlen = -1;
			}
		} else {
			u_short swsum;

			sum = *(u_char *)w << 8;
			mlen--;
			w = (u_short *)(1 + (int)w);
			swsum = ocsum(w, mlen>>1);
			swab((char *)&swsum, (char *)&swsum, sizeof swsum);
			sum += swsum;
			w += mlen>>1;
			if (mlen & 1)
				sum += *(u_char *)w;
		}
	}
	sum = (sum & 0xFFFF) + (sum >> 16);
	sum = (sum & 0xFFFF) + (sum >> 16);
	sum = (~sum) & 0xFFFF;
	return (sum);
}
