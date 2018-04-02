/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_en.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Structure of a Ethernet header.
 */
struct	en_header {
	u_char	en_shost;
	u_char	en_dhost;
	u_short	en_type;
};

#define	ENPUP_PUPTYPE	0x0400		/* PUP protocol */
#define	ENPUP_IPTYPE	0x0800		/* IP protocol */

/*
 * The ENPUP_NTRAILER packet types starting at ENPUP_TRAIL have
 * (type-ENPUP_TRAIL)*512 bytes of data followed
 * by a PUP type (as given above) and then the (variable-length) header.
 */
#define	ENPUP_TRAIL	0x1000		/* Trailer PUP */
#define	ENPUP_NTRAILER	16
