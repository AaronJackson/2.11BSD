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
 *	@(#)tcp_debug.c	7.4 (2.11BSD) 2022/1/21
 */

#ifdef	TCPDEBUG
#define	TCPTIMERS
#define	TANAMES
#define PRUREQUESTS
#define TCPSTATES
#endif

#include "param.h"
#include "systm.h"
#include "mbuf.h"
#include "socket.h"
#include "socketvar.h"
#include "protosw.h"
#include "errno.h"

#include "../net/route.h"
#include "../net/if.h"

#include "domain.h"
#include "in.h"
#include "in_pcb.h"
#include "in_systm.h"
#include "ip.h"
#include "ip_var.h"
#include "tcp.h"
#include "tcp_fsm.h"
#include "tcp_seq.h"
#include "tcp_timer.h"
#include "tcp_var.h"
#include "tcpip.h"
#include "tcp_debug.h"

#ifdef	TCPDEBUG
int	tcpconsdebug = 0;
#endif

/*
 * Tcp debug routines
 */
tcp_trace(act, ostate, tp, ti, req)
	short act, ostate;
	struct tcpcb *tp;
	struct tcpiphdr *ti;
	int req;
{
	tcp_seq seq, ack;
	int len, flags;
	struct tcp_debug *td = &tcp_debug[tcp_debx++];

	if (tcp_debx == TCP_NDEBUG)
		tcp_debx = 0;
	td->td_time = iptime();
	td->td_act = act;
	td->td_ostate = ostate;
	td->td_tcb = (caddr_t)tp;
	if (tp)
		td->td_cb = *tp;
	else
		bzero((caddr_t)&td->td_cb, sizeof (*tp));
	if (ti)
		td->td_ti = *ti;
	else
		bzero((caddr_t)&td->td_ti, sizeof (*ti));
	td->td_req = req;
#ifdef	TCPDEBUG
	if (tcpconsdebug == 0)
		return;
	if (tp)
		printf("%x %s:", tp, tcpstates[ostate]);
	else
		printf("???????? ");
	printf("%s ", tanames[act]);
	switch (act) {

	case TA_INPUT:
	case TA_OUTPUT:
	case TA_DROP:
		if (ti == 0)
			break;
		seq = ti->ti_seq;
		ack = ti->ti_ack;
		len = ti->ti_len;
		if (act == TA_OUTPUT) {
			seq = ntohl(seq);
			ack = ntohl(ack);
			len = ntohs((u_short)len);
		}
		if (act == TA_OUTPUT)
			len -= sizeof (struct tcphdr);
		if (len)
			printf("[%lx..%lx)", seq, seq+len);
		else
			printf("%lx", seq);
		printf("@%lx, urp=%lx", ack, ti->ti_urp);
		flags = ti->ti_flags;
		if (flags) {
#ifndef lint
			char *cp = "<";
#define pf(f) { if (ti->ti_flags&TH_ ## f) { printf("%s%s", cp, "f"); cp = ","; } }
			pf(SYN); pf(ACK); pf(FIN); pf(RST); pf(PUSH); pf(URG);
#endif
			printf(">");
		}
		break;

	case TA_USER:
		printf("%s", prurequests[req&0xff]);
		if ((req & 0xff) == PRU_SLOWTIMO)
			printf("<%s>", tcptimers[req>>8]);
		break;
	}
	if (tp)
		printf(" -> %s", tcpstates[tp->t_state]);
	/* print out internal state of tp !?! */
	printf("\n");
	if (tp == 0)
		return;
	printf("\trcv_(nxt,wnd,up) (%lx,%x,%x) snd_(una,nxt,max) (%lx,%lx,%lx)\n",
	    tp->rcv_nxt, tp->rcv_wnd, tp->rcv_up, tp->snd_una, tp->snd_nxt,
	    tp->snd_max);
	printf("\tsnd_(wl1,wl2,wnd) (%lx,%lx,%x)\n",
	    tp->snd_wl1, tp->snd_wl2, tp->snd_wnd);
#endif /* TCPDEBUG */
}
