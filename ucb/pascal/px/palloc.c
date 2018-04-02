#include "0x.h"
#include "E.h"

extern char *sbrk(), *brk();

alloc(need)
	char *need;
{
	register cnt, *wp;
	register char *have;

	need = (char *)((unsigned)(need+1) &~ 1);
	if ((have=high-memptr) < need) {
		if (sbrk(need > have + 1024 ? need-have:1024) == -1)
			error(EOUTOFMEM);
		high = sbrk(0);
	}
	wp = memptr;
	cnt = ((unsigned)need >> 1) & 077777;
	do {
		*wp++ = 0;
	} while (--cnt);
	wp = memptr;
	memptr += (unsigned)need;
	stklim();
	return(wp);
}

setmem()
{
	high = bottmem = memptr = sbrk(0);
	stklim();
}

free(cptr)
	char *cptr;
{
}

stklim()
{
	maxstk = (char *)((((unsigned)memptr + 07777) &~ 07777) + 512);
}
