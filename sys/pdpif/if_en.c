/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_en.c	1.3 (2.11BSD GTE) 1997/1/19
 */

#include "en.h"
#if	NEN > 0

/*
 * Xerox prototype (3 Mb) Ethernet interface driver.
 */

#include "param.h"
#include "systm.h"
#include "mbuf.h"
#include "buf.h"
#include "domain.h"
#include "protosw.h"
#include "socket.h"
#include "pdpuba/ubavar.h"
#include "if_enreg.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "net/netisr.h"
#include "net/if.h"
#include "pdpif/if_en.h"
#include "pdpif/if_uba.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/in_var.h"
#include "net/route.h"
#include "errno.h"

#define	ENMTU	(1024+512)
#define	ENMRU	(1024+512+16)		/* 16 is enough to receive trailer */

int	enprobe(), enattach(), enrint(), enxint(), encollide();
struct	uba_device *eninfo[NEN];
u_short enstd[] = { 0 };
struct	uba_driver endriver =
	{ enprobe, 0, enattach, 0, enstd, "en", eninfo };
#define	ENUNIT(x)	minor(x)

int	eninit(),enoutput();

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * es_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 * We also have, for each interface, a UBA interface structure, which
 * contains information about the UNIBUS resources held by the interface:
 * map registers, buffered data paths, etc.  Information is cached in this
 * structure for use by the if_uba.c routines in running the interface
 * efficiently.
 */
struct	en_softc {
	struct	ifnet es_if;		/* network-visible interface */
	struct	ifuba es_ifuba;		/* UNIBUS resources */
	short	es_delay;		/* current output delay */
	short	es_mask;		/* mask for current output delay */
	short	es_lastx;		/* host last transmitted to */
	short	es_oactive;		/* is output active? */
	short	es_olen;		/* length of last output */
} en_softc[NEN];

/*
 * Do output DMA to determine interface presence and
 * interrupt vector.  DMA is too short to disturb other hosts.
 */
enprobe(reg)
	caddr_t reg;
{
	register int br, cvec;		/* r11, r10 value-result */
	register struct endevice *addr = (struct endevice *)reg;

#ifndef	pdp11
#ifdef lint
	br = 0; cvec = br; br = cvec;
	enrint(0); enxint(0); encollide(0);
#endif
	addr->en_istat = 0;
	addr->en_owc = -1;
	addr->en_oba = 0;
	addr->en_ostat = EN_IEN|EN_GO;
	DELAY(100000L);
	addr->en_ostat = 0;
#ifdef ECHACK
	br = 0x16;
#endif
#endif
	return (1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
enattach(ui)
	struct uba_device *ui;
{
	register struct en_softc *es = &en_softc[ui->ui_unit];
	register struct sockaddr_in *sin;

	es->es_if.if_unit = ui->ui_unit;
	es->es_if.if_name = "en";
	es->es_if.if_mtu = ENMTU;
	es->es_if.if_flags = IFF_BROADCAST;
	es->es_if.if_init = eninit;
	es->es_if.if_output = enoutput;
	es->es_ifuba.ifu_flags = UBA_NEEDBDP | UBA_NEED16 | UBA_CANTWAIT;
	if_attach(&es->es_if);
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
eninit(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];
	register struct uba_device *ui = eninfo[unit];
	register struct endevice *addr;
	int s;

	if (if_ubainit(&es->es_ifuba, ui->ui_ubanum,
	    sizeof (struct en_header), (int)btoc(ENMRU)) == 0) { 
		printf("en%d: can't initialize\n", unit);
		es->es_if.if_flags &= ~IFF_UP;
		return;
	}
	addr = (struct endevice *)ui->ui_addr;
	addr->en_istat = addr->en_ostat = 0;

	/*
	 * Hang a receive and start any
	 * pending writes by faking a transmit complete.
	 */
	s = splimp();
	addr->en_iba = es->es_ifuba.ifu_r.ifrw_info;
	addr->en_iwc = -(sizeof (struct en_header) + ENMRU) >> 1;
	addr->en_istat = EN_IEN|EN_GO;
	es->es_oactive = 1;
	es->es_if.if_flags |= IFF_UP;
	enxint(unit);
	splx(s);
	if_rtinit(&es->es_if, RTF_UP);
}

int	enalldelay = 0;
int	enlastdel = 50;
int	enlastmask = (~0) << 5;

/*
 * Start or restart output on interface.
 * If interface is already active, then this is a retransmit
 * after a collision, and just restuff registers and delay.
 * If interface is not already active, get another datagram
 * to send off of the interface queue, and map it to the interface
 * before starting the output.
 */
enstart(dev)
	dev_t dev;
{
        int unit = ENUNIT(dev);
	struct uba_device *ui = eninfo[unit];
	register struct en_softc *es = &en_softc[unit];
	register struct endevice *addr;
	struct mbuf *m;
	int dest;

	if (es->es_oactive)
		goto restart;

	/*
	 * Not already active: dequeue another request
	 * and map it to the UNIBUS.  If no more requests,
	 * just return.
	 */
	IF_DEQUEUE(&es->es_if.if_snd, m);
	if (m == 0) {
		es->es_oactive = 0;
		return;
	}
	dest = mtod(m, struct en_header *)->en_dhost;
	es->es_olen = if_wubaput(&es->es_ifuba, m);

	/*
	 * Ethernet cannot take back-to-back packets (no
	 * buffering in interface.  To help avoid overrunning
	 * receivers, enforce a small delay (about 1ms) in interface:
	 *	* between all packets when enalldelay
	 *	* whenever last packet was broadcast
	 *	* whenever this packet is to same host as last packet
	 */
	if (enalldelay || es->es_lastx == 0 || es->es_lastx == dest) {
		es->es_delay = enlastdel;
		es->es_mask = enlastmask;
	}
	es->es_lastx = dest;

restart:
	addr = (struct endevice *)ui->ui_addr;
	addr->en_oba = (int)es->es_ifuba.ifu_w.ifrw_info;
	addr->en_odelay = es->es_delay;
	addr->en_owc = -((es->es_olen + 1) >> 1);
	addr->en_ostat = EN_IEN|EN_GO;
	es->es_oactive = 1;
}

/*
 * Ethernet interface transmitter interrupt.
 * Start another output if more data to send.
 */
enxint(unit)
	int unit;
{
	register struct uba_device *ui = eninfo[unit];
	register struct en_softc *es = &en_softc[unit];
	register struct endevice *addr = (struct endevice *)ui->ui_addr;

	if (es->es_oactive == 0)
		return;
	if (es->es_mask && (addr->en_ostat&EN_OERROR)) {
		es->es_if.if_oerrors++;
		endocoll(unit);
		return;
	}
	es->es_if.if_opackets++;
	es->es_oactive = 0;
	es->es_delay = 0;
	es->es_mask = ~0;
	if (es->es_ifuba.ifu_xtofree) {
		m_freem(es->es_ifuba.ifu_xtofree);
		es->es_ifuba.ifu_xtofree = 0;
	}
	if (es->es_if.if_snd.ifq_head == 0) {
		es->es_lastx = 256;		/* putatively illegal */
		return;
	}
	enstart(unit);
}

/*
 * Collision on ethernet interface.  Do exponential
 * backoff, and retransmit.  If have backed off all
 * the way print warning diagnostic, and drop packet.
 */
encollide(unit)
	int unit;
{
	struct en_softc *es = &en_softc[unit];

	es->es_if.if_collisions++;
	if (es->es_oactive == 0)
		return;
	endocoll(unit);
}

endocoll(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];

	/*
	 * Es_mask is a 16 bit number with n low zero bits, with
	 * n the number of backoffs.  When es_mask is 0 we have
	 * backed off 16 times, and give up.
	 */
	if (es->es_mask == 0) {
		printf("en%d: send error\n", unit);
		enxint(unit);
		return;
	}
	/*
	 * Another backoff.
	 */
	es->es_mask <<= 1;
	es->es_delay = ffs(es->es_mask);
	enstart(unit);
}

#ifdef	notdef
struct	sockaddr_pup pupsrc = { AF_PUP };
struct	sockaddr_pup pupdst = { AF_PUP };
struct	sockproto pupproto = { PF_PUP };
#endif
/*
 * Ethernet interface receiver interrupt.
 * If input error just drop packet.
 * Otherwise purge input buffered data path and examine 
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
enrint(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];
	struct endevice *addr = (struct endevice *)eninfo[unit]->ui_addr;
	register struct en_header *en;
    	struct mbuf *m;
	int len, plen; short resid;
	register struct ifqueue *inq;
	int off;

	es->es_if.if_ipackets++;

	if (addr->en_istat&EN_IERROR) {
		es->es_if.if_ierrors++;
		goto setup;
	}

	/*
	 * Calculate input data length.
	 * Get pointer to ethernet header (in input buffer).
	 * Deal with trailer protocol: if type is PUP trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	resid = addr->en_iwc;
	if (resid)
		resid |= 0176000;
	len = (((sizeof (struct en_header) + ENMRU) >> 1) + resid) << 1;
	len -= sizeof (struct en_header);
	if (len > ENMRU)
		goto setup;			/* sanity */
	en = (struct en_header *)(es->es_ifuba.ifu_r.ifrw_info);
#define	endataaddr(en, off, type)	((type)(((caddr_t)((en)+1)+(off))))
	if (en->en_type >= ENPUP_TRAIL &&
	    en->en_type < ENPUP_TRAIL+ENPUP_NTRAILER) {
		off = (en->en_type - ENPUP_TRAIL) * 512;
		if (off > ENMTU)
			goto setup;		/* sanity */
		en->en_type = *endataaddr(en, off, u_short *);
		resid = *(endataaddr(en, off+2, u_short *));
		if (off + resid > len)
			goto setup;		/* sanity */
		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		goto setup;
	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; if_rubaget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = if_rubaget(&es->es_ifuba, len, off, &es->es_if);
	if (m == 0)
		goto setup;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (en->en_type) {

#ifdef INET
	case ENPUP_IPTYPE:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;
#endif
#ifdef PUP
	case ENPUP_PUPTYPE: {
		struct pup_header *pup = mtod(m, struct pup_header *);

		pupproto.sp_protocol = pup->pup_type;
		pupdst.spup_addr = pup->pup_dport;
		pupsrc.spup_addr = pup->pup_sport;
		raw_input(m, &pupproto, (struct sockaddr *)&pupsrc,
		  (struct sockaddr *)&pupdst);
		goto setup;
	}
#endif
	default:
		m_freem(m);
		goto setup;
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
	} else
		IF_ENQUEUE(inq, m);

setup:
	/*
	 * Reset for next packet.
	 */
	addr->en_iba = es->es_ifuba.ifu_r.ifrw_info;
	addr->en_iwc = -(sizeof (struct en_header) + ENMRU) >> 1;
	addr->en_istat = EN_IEN|EN_GO;
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
enoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, dest, s, error;
	register struct mbuf *m = m0;
	register struct en_header *en;
	register int off;

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		dest = ((struct sockaddr_in *)dst)->sin_addr.s_addr;
		if (dest & 0x00ffff00) {
			error = EPERM;		/* ??? */
			goto bad;
		}
		dest = (dest >> 24) & 0xff;
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		if (off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ENPUP_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = ENPUP_IPTYPE;
			*(mtod(m, u_short *) + 1) = m->m_len;
			goto gottrailertype;
		}
		type = ENPUP_IPTYPE;
		off = 0;
		goto gottype;
#endif
#ifdef PUP
	case AF_PUP:
		dest = ((struct sockaddr_pup *)dst)->spup_addr.pp_host;
		type = ENPUP_PUPTYPE;
		off = 0;
		goto gottype;
#endif

	default:
		printf("en%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
	}

gottrailertype:
	/*
	 * Packet to be sent as trailer: move first packet
	 * (control information) to end of chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct en_header) > m->m_off) {
		m = m_get(M_DONTWAIT);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct en_header);
	} else {
		m->m_off -= sizeof (struct en_header);
		m->m_len += sizeof (struct en_header);
	}
	en = mtod(m, struct en_header *);
	en->en_dhost = dest;
	en->en_type = htons((u_short)type);

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		error = ENOBUFS;
		goto qfull;
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	if (en_softc[ifp->if_unit].es_oactive == 0)
		enstart(ifp->if_unit);
	splx(s);
	return (0);
qfull:
	m0 = m;
	splx(s);
bad:
	m_freem(m0);
	return (error);
}
#endif	NEN > 0
