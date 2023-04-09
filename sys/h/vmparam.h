/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vmparam.h	7.2 (2.11BSD) 2020/3/3
 */

/*
 * Machine dependent constants
 */
#ifdef KERNEL
#include "../machine/vmparam.h"
#else
#include <machine/vmparam.h>
#endif

/*
 * CTL_VM identifiers
 */
#define	VM_METER	1		/* struct vmmeter */
#define	VM_LOADAVG	2		/* struct loadavg */
#define	VM_SWAPMAP	3		/* struct mapent _swapmap[] */
#define	VM_COREMAP	4		/* struct mapent _coremap[] */
#define VM_NSWAP	5		/* int nswap */
#define	VM_MAXID	6		/* number of valid vm ids */

#ifndef	KERNEL
#define CTL_VM_NAMES { \
	{ 0, 0 }, \
	{ "vmmeter", CTLTYPE_STRUCT }, \
	{ "loadavg", CTLTYPE_STRUCT }, \
	{ "swapmap", CTLTYPE_STRUCT }, \
	{ "coremap", CTLTYPE_STRUCT }, \
	{ "nswap", CTLTYPE_INT }, \
}
#endif
