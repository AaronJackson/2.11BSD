/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)tcp.h	7.4.1.1 (Berkeley) 2/7/88
 */
#ifndef BYTE_ORDER
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax) */
#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#ifdef vax
#define	BYTE_ORDER	LITTLE_ENDIAN
#else
#ifdef pdp11
#define	BYTE_ORDER	PDP_ENDIAN
#else
#define	BYTE_ORDER	BIG_ENDIAN	/* mc68000, tahoe, most others */
#endif
#endif
#endif BYTE_ORDER

typedef	u_long	tcp_seq;
/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	u_short	th_sport;		/* source port */
	u_short	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN
	u_char	th_off:4,		/* data offset */
		th_x2:4;		/* (unused) */
#endif
#if BYTE_ORDER == PDP_ENDIAN
	u_int	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#endif
	u_char	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
	u_short	th_win;			/* window */
	u_short	th_sum;			/* checksum */
	u_short	th_urp;			/* urgent pointer */
};

#define	TCPOPT_EOL	0
#define	TCPOPT_NOP	1
#define	TCPOPT_MAXSEG	2

/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 is probably more convenient.
 */
#ifdef	lint
#define	TCP_MSS	536
#else
#ifndef IP_MSS
#define	IP_MSS	576
#endif
#define	TCP_MSS	MIN(512, IP_MSS - sizeof (struct tcpiphdr))
#endif

/*
 * User-settable options (used with setsockopt).
 */
#define	TCP_NODELAY	0x01	/* don't delay send to coalesce packets */
#define	TCP_MAXSEG	0x02	/* set maximum segment size */
