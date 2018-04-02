/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_uba.h	1.2 (2.11BSD GTE) 12/24/92
 */

/*
 * Structure and routine definitions
 * for UNIBUS network interfaces.
 */

#define	IF_MAXNUBAMR	10
/*
 * Each interface has one of these structures giving information
 * about UNIBUS resources held by the interface.
 *
 * We hold IF_NUBAMR map registers for datagram data, starting
 * at ifr_mr.  Map register ifr_mr[-1] maps the local network header
 * ending on the page boundary.  Bdp's are reserved for read and for
 * write, given by ifr_bdp.  The prototype of the map register for
 * read and for write is saved in ifr_proto.
 *
 * When write transfers are not full pages on page boundaries we just
 * copy the data into the pages mapped on the UNIBUS and start the
 * transfer.  If a write transfer is of a (1024 byte) page on a page
 * boundary, we swap in UNIBUS pte's to reference the pages, and then
 * remap the initial pages (from ifu_wmap) when the transfer completes.
 *
 * When read transfers give whole pages of data to be input, we
 * allocate page frames from a network page list and trade them
 * with the pages already containing the data, mapping the allocated
 * pages to replace the input pages for the next UNIBUS data input.
 */
struct	ifuba {
	short	ifu_hlen;			/* local net header length */
	struct ifrw {
		u_int   ifrw_click;             /* MMU click address */
		u_long  ifrw_info;              /* value from ubaalloc */
	} ifu_r, ifu_w;
	short   ifu_flags;
	struct	mbuf *ifu_xtofree;		/* pages being dma'd out */
};

#ifdef SUPERVISOR
struct	mbuf *if_rubaget();
#endif
#if defined(KERNEL) && defined(INET)
#define	ubarelse(a,b)
#define	useracc(a,c,m)		(1)
ubadr_t	uballoc(), ubmalloc();
#endif
