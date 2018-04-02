/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_un.h	1.1 (2.10BSD Berkeley) 12/1/86
 */

/*
 * Structure of a Ungermann-Bass datagram header.
 */

struct un_header {
	u_short	un_length;
	u_char	un_type;
	u_char	un_control;
	u_short	un_dnet;
	u_short	un_dniu;
	u_char	un_dport;
	u_char	un_dtype;
	u_short	un_snet;
	u_short	un_sniu;
	u_char	un_sport;
	u_char	un_stype;
	u_char	un_option;
	u_char	un_bcc;
	u_short	un_ptype;	/* protocol type */
};

/*
 * Protocol types
 */

#define	UNTYPE_INQUIRE		1	/* inquire - "Who am I?" */
#define	UNTYPE_IP		2	/* Internet Protocol */
