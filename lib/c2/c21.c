#
/*
 * C object code improver-- second part
 *
 *      @(#) c21.c 2.0  (2.11BSD) 2022/1/21
 */

#include "c2.h"

rmove()
{
	register struct node *p;
	register int r;
	register  r1, flt;

	for (p=first.forw; p!=0; p = p->forw) {
	flt = 0;
	switch (p->op) {

	case MOVF:
	case MOVFO:
	case MOVOF:
		flt = NREG;

	case MOV:
		if (p->subop==BYTE)
			goto dble;
		dualop(p);
		if ((r = findrand(regs[RT1], flt)) >= 0) {
			if (r == flt+isreg(regs[RT2]) && p->forw->op!=CBR
			   && p->forw->op!=SXT
			   && p->forw->op!=CFCC) {
				p->forw->back = p->back;
				p->back->forw = p->forw;
				redunm++;
				nchange++;
				continue;
			}
		}
		if (equstr(regs[RT1], "$0")) {
			p->op = CLR;
			strcpy(regs[RT1], regs[RT2]);
			regs[RT2][0] = 0;
			p->code = copy(1, regs[RT1]);
			nchange++;
			goto sngl;
		}
		repladdr(p, 0, flt);
		r = isreg(regs[RT1]);
		r1 = isreg(regs[RT2]);
		dest(regs[RT2], flt);
		if (r >= 0)
			if (r1 >= 0)
				savereg(r1+flt, regs[r+flt]);
			else
				savereg(r+flt, regs[RT2]);
		else
			if (r1 >= 0)
				savereg(r1+flt, regs[RT1]);
			else
				setcon(regs[RT1], regs[RT2]);
		source(regs[RT1]);
		setcc(regs[RT2]);
		continue;

	case ADDF:
	case SUBF:
	case DIVF:
	case MULF:
		flt = NREG;
		goto dble;

	case ADD:
	case SUB:
	case BIC:
	case BIS:
	case MUL:
	case DIV:
	case ASH:
	dble:
		dualop(p);
		if (p->op==BIC && (equstr(regs[RT1], "$-1") || equstr(regs[RT1], "$177777"))) {
			p->op = CLR;
			strcpy(regs[RT1], regs[RT2]);
			regs[RT2][0] = 0;
			p->code = copy(1, regs[RT1]);
			nchange++;
			goto sngl;
		}
		if ((p->op==BIC || p->op==BIS) && equstr(regs[RT1], "$0")) {
			if (p->forw->op!=CBR) {
				p->back->forw = p->forw;
				p->forw->back = p->back;
				nchange++;
				continue;
			}
		}
/*
 * the next block of code looks for the sequences (which extract the
 * high byte of a word or the low byte respectively):
 *	ash $-10,r
 *	bic $-400,r
 * or 
 *	mov natural,r
 *	bic $-400,r
 * and transforms them into:
 *	clrb r
 *	swab r
 * or
 *	clr r
 *	bisb natural,r
 * These constructs occur often enough in the kernel (dealing with major/minor 
 * device numbers, etc) it's worth a little extra work at compile time.
*/
		if (p->op == BIC && (equstr(regs[RT1],"$-400") || 
			 equstr(regs[RT1],"$-177400"))) {
			if (p->back->op == ASH) {
				r = isreg(regs[RT2]);
				dualop(p->back);
				if ((equstr(regs[RT1], "$-10") || 
				     equstr(regs[RT1], "$177770")) && 
				    r == isreg(regs[RT2])) {
					strcpy(regs[RT1], regs[RT2]);
					regs[RT2][0] = 0;
					p->back->op = CLR;
					p->back->subop = BYTE;
					p->back->code = copy(1, regs[RT1]);
					p->op = SWAB;
					p->code = copy(1, regs[RT1]);
					nchange++;
					goto sngl;
				}
			}
			else if (p->back->op == MOV && p->forw->op != CBR) {
				char temp[50];

				r = isreg(regs[RT2]);
				if (r < 0 && !xnatural(regs[RT2]))
					goto out;
				strcpy(temp, regs[RT2]);
				dualop(p->back);
				if (isreg(regs[RT2]) == r && natural(regs[RT1])) {
			 	    if (r < 0 && (!xnatural(regs[RT2]) || !equstr(temp, regs[RT2])))
					goto out;
/*
 * XXX - the sequence "movb rN,rN; bic $-400,rN" can not be transformed
 * because the 'clr' would lose all information about 'rN'.  The best that can 
 * be done is to remove the 'movb' instruction and leave the 'bic'.
*/
				    if (isreg(regs[RT1]) == r && r >= 0) {
					    p = p->back;
					    p->forw->back = p->back;
					    p->back->forw = p->forw;
					    nchange++;
					    continue;
				    }
				    dest(regs[RT1], flt);
				    p->back->op = CLR;
				    p->back->subop = 0;
				    p->back->code = copy(1, regs[RT2]);
				    p->op = BIS;
				    p->subop = BYTE;
				    strcat(regs[RT1], ",");
				    p->code = copy(2, regs[RT1], regs[RT2]);
				    nchange++;
				}
			}
out:		dualop(p);	/* restore banged up parsed operands */
		}
		repladdr(p, 0, flt);
		source(regs[RT1]);
		dest(regs[RT2], flt);
		if (p->op==DIV && (r = isreg(regs[RT2]))>=0)
			regs[r|1][0] = 0;
		switch	(p->op)
			{
			case	ADD:
			case	SUB:
			case	BIC:
			case	BIS:
			case	ASH:
				setcc(regs[RT2]);
				break;
			default:
				ccloc[0] = 0;
			}
		continue;

	case SXT:
		singop(p);
		if (p->forw->op == CLR && p->forw->subop != BYTE &&
			xnatural(regs[RT1]) && !strcmp(p->code, p->forw->code)){
			p->forw->back = p->back;
			p->back->forw = p->forw;
			nchange++;
			continue;
		}
		goto sngl;
	case CLRF:
	case NEGF:
		flt = NREG;

	case CLR:
	case COM:
	case INC:
	case DEC:
	case NEG:
	case ASR:
	case ASL:
	case SWAB:
		singop(p);
	sngl:
		dest(regs[RT1], flt);
		if (p->op==CLR && flt==0)
			{
			if ((r = isreg(regs[RT1])) >= 0)
				savereg(r, "$0");
			else
				setcon("$0", regs[RT1]);
			ccloc[0] = 0;
			}
		else
			setcc(regs[RT1]);
		continue;

	case TSTF:
		flt = NREG;

	case TST:
		singop(p);
		repladdr(p, 0, flt);
		source(regs[RT1]);
		if (p->back->op == TST && !flt && not_sp(regs[RT1])) {
			char rt1[MAXCPS + 2];
			strcpy(rt1, regs[RT1]);
			singop(p->back);
			if (!strcmp("(sp)+", regs[RT1])) {
				p->back->subop = p->subop;
				p->back->forw = p->forw;
				p->forw->back = p->back;
				p = p->back;
				p->op = MOV;
				p->code = copy(2, rt1, ",(sp)+");
				nrtst++;
				nchange++;
				continue;
			}
		singop(p);
		}
		if (p->back->op == MOV && p->back->subop == BYTE) {
			dualop(p->back);
			setcc(regs[RT2]);
			singop(p);
		}
		if (equstr(regs[RT1], ccloc) && p->subop == p->back->subop) {
			p->back->forw = p->forw;
			p->forw->back = p->back;
			p = p->back;
			nrtst++;
			nchange++;
		}
		else
			setcc(regs[RT1]); /* XXX - double TST in a row */
		continue;

	case CMPF:
		flt = NREG;

	case CMP:
	case BIT:
		dualop(p);
		source(regs[RT1]);
		source(regs[RT2]);
		if(p->op==BIT) {
			if (equstr(regs[RT1], "$-1") || equstr(regs[RT1], "$177777")) {
				p->op = TST;
				strcpy(regs[RT1], regs[RT2]);
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			} else if (equstr(regs[RT2], "$-1") || equstr(regs[RT2], "$177777")) {
				p->op = TST;
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			}
			if (equstr(regs[RT1], "$0")) {
				p->op = TST;
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			} else if (equstr(regs[RT2], "$0")) {
				p->op = TST;
				strcpy(regs[RT1], regs[RT2]);
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			}
		}
		repladdr(p, 1, flt);
		ccloc[0] = 0;
		continue;

	case CBR:
		r = -1;
		if (p->back->op==TST || p->back->op==CMP) {
			if (p->back->op==TST) {
				singop(p->back);
				savereg(RT2, "$0");
			} else
				dualop(p->back);
			if (equstr(regs[RT1], regs[RT2])
			 && natural(regs[RT1]) && natural(regs[RT2]))
				r = compare(p->subop, "$1", "$1");
			else
				r = compare(p->subop, findcon(RT1), findcon(RT2));
			if (r==0) {
				if (p->forw->op==CBR
				  || p->forw->op==SXT
				  || p->forw->op==CFCC) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
				} else {
					p->back->back->forw = p->forw;
					p->forw->back = p->back->back;
				}
				decref(p->ref);
				p = p->back->back;
				nchange++;
			} else if (r>0) {
				p->op = JBR;
				p->subop = 0;
				p->back->back->forw = p;
				p->back = p->back->back;
				p = p->back;
				nchange++;
			}
/*
 * If the instruction prior to the conditional branch was a 'tst' then
 * save the condition code status.  The C construct:
 * 		if (x) 
 *		   if (x > 0)
 * generates "tst _x; jeq ...; tst _x; jmi ...;jeq ...".  The code below removes
 * the second "tst _x", leaving "tst _x; jeq ...;jmi ...; jeq ...".
*/
			if (p->back->op == TST) {
				singop(p->back);
				setcc(regs[RT1]);
				break;
			}
		}
/*
 * If the previous instruction was also a conditional branch then
 * attempt to merge the two into a single branch.
*/
		if (p->back->op == CBR)
			fixupbr(p);
	case CFCC:
		ccloc[0] = 0;
		continue;

/*
 * Unrecognized (unparsed) instructions, assignments (~foo=r2), and
 * data arrive here.  In order to prevent throwing away information
 * about register contents just because a local assignment is done
 * we check for the first character being a tilde.
*/
	case 0:
		if (p->code[0] != '~')
			clearreg();
		continue;

	case JBR:
		redunbr(p);

	default:
		clearreg();
	}
	}
}

/*
 * This table is used to convert two branches to the same label after a 
 * 'tst' (which clears the C and V condition codes) into a single branch.
 * Entries which translate to JBR could eventually cause the 'tst' instruction 
 * to be eliminated as well, but that can wait for now.  There are unused or
 * impossible combinations ('tst' followed by 'jlo' for example.  since
 * 'tst' clears C it makes little sense to 'jlo/bcs') in the table, it 
 * would have cost more in code to remove them than the entries themselves.
 *
 * Example:  "tst _x; jmi L3; jeq L3".  Find the row for 'jmi', then look
 * at the column for 'jeq', the resulting "opcode" is 'jle'.
*/
	char	brtable[12][12] = {
	/* jeq  jne  jle  jge  jlt  jgt  jlo  jhi  jlos jhis jpl  jmi */
/* jeq */ {JEQ ,JBR ,JLE ,JGE ,JLE ,JGE ,JEQ ,JBR ,JEQ ,JBR ,JGE ,JLE},
/* jne */ {JBR ,JNE ,JBR ,JBR ,JNE ,JNE ,JNE ,JNE ,JBR ,JBR ,JBR ,JNE},
/* jle */ {JLE ,JBR ,JLE ,JBR ,JLE ,JBR ,JLE ,JBR ,JLE ,JBR ,JBR ,JLE},
/* jge */ {JGE ,JBR ,JBR ,JGE ,JBR ,JGE ,JGE ,JBR ,JGE ,JBR ,JGE ,JBR},
/* jlt */ {JLE ,JNE ,JLE ,JBR ,JLT ,JNE ,JLT ,JBR ,JLE ,JBR ,JBR ,JLT},
/* jgt */ {JGE ,JNE ,JBR ,JGE ,JNE ,JGT ,JGT ,JGT ,JBR ,JGE ,JGE ,JNE},
/* jlo */ {JEQ ,JNE ,JLE ,JGE ,JLT ,JGT ,JLO ,JHI ,JLOS,JHIS,JPL ,JMI},
/* jhi */ {JBR ,JNE ,JBR ,JBR ,JNE ,JNE ,JNE ,JNE ,JBR ,JBR ,JBR ,JNE},
/* jlos*/ {JEQ ,JBR ,JLE ,JGE ,JLE ,JGE ,JLE ,JBR ,JEQ ,JBR ,JGE ,JLE},
/* jhis*/ {JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR ,JBR},
/* jpl */ {JGE ,JBR ,JBR ,JGE ,JBR ,JGE ,JGE ,JBR ,JGE ,JBR ,JGE ,JBR},
/* jmi */ {JLE ,JNE ,JLE ,JBR ,JLT ,JNE ,JLT ,JNE ,JLE ,JLT ,JBR ,JLT}
	  };

fixupbr(p)
	register struct node *p;
{
	register struct node *p1, *p2;
	int op;

	p1 = p->back;
	p2 = p1->back;
	if (p->labno != p1->labno)
		return;
	if (p2->op != TST) {
		if (p2->op == CBR && p2->back->op == TST)
			goto ok;
		return;
	}
ok:	p->subop = brtable[p->subop][p1->subop];
	nchange++;
	nredunj++;
	p2->forw = p;
	p->back = p1->back;
	}

jumpsw()
{
	register struct node *p, *p1;
	register t;
	register struct node *tp;
	int nj;

	t = 0;
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++t;
	for (p=first.forw; p!=0; p = p1) {
		p1 = p->forw;
		if (p->op == CBR && p1->op==JBR && p->ref && p1->ref
		 && abs(p->refc - p->ref->refc) > abs(p1->refc - p1->ref->refc)) {
			if (p->ref==p1->ref)
				continue;
			p->subop = revbr[p->subop];
			tp = p1->ref;
			p1->ref = p->ref;
			p->ref = tp;
			t = p1->labno;
			p1->labno = p->labno;
			p->labno = t;
			nrevbr++;
			nj++;
		}
	}
	return(nj);
}

addsob()
{
	register struct node *p, *p1;

	for (p = &first; (p1 = p->forw)!=0; p = p1) {
		if (p->op==DEC && isreg(p->code)>=0
		 && p1->op==CBR && p1->subop==JNE) {
			if (p->refc < p1->ref->refc)
				continue;
			if (toofar(p1))
				continue;
			p->labno = p1->labno;
			p->op = SOB;
			p->subop = 0;
			p1->forw->back = p;
			p->forw = p1->forw;
			nsob++;
		}
	}
}

toofar(p)
struct node *p;
{
	register struct node *p1;
	int len;

	len = 0;
	for (p1 = p->ref; p1 && p1!=p; p1 = p1->forw)
		len += ilen(p1);
	if (len < 128)
		return(0);
	return(1);
}

ilen(p)
register struct node *p;
{

	switch (p->op) {
	case LABEL:
	case DLABEL:
	case TEXT:
	case EROU:
	case EVEN:
		return(0);

	case CBR:
		return(6);

	default:
		dualop(p);
		return(2 + adrlen(regs[RT1]) + adrlen(regs[RT2]));
	}
}

adrlen(s)
register char *s;
{
	if (*s == 0)
		return(0);
	if (*s=='r')
		return(0);
	if (*s=='(' && *(s+1)=='r')
		return(0);
	if (*s=='-' && *(s+1)=='(')
		return(0);
	return(2);
}

abs(x)
register int x;
{
	return(x<0? -x: x);
}

equop(ap1, p2)
struct node *ap1, *p2;
{
	register char *cp1, *cp2;
	register struct node *p1;

	p1 = ap1;
	if (p1->op!=p2->op || p1->subop!=p2->subop)
		return(0);
	if (p1->op>0 && p1->op<MOV)
		return(0);
	cp1 = p1->code;
	cp2 = p2->code;
	if (cp1==0 && cp2==0)
		return(1);
	if (cp1==0 || cp2==0)
		return(0);
	while (*cp1 == *cp2++)
		if (*cp1++ == 0)
			return(1);
	return(0);
}

decref(p)
register struct node *p;
{
	if (--p->refc <= 0) {
		nrlab++;
		p->back->forw = p->forw;
		p->forw->back = p->back;
	}
}

struct node *
nonlab(p)
register struct node *p;
{
	CHECK(10);
	while (p && p->op==LABEL)
		p = p->forw;
	return(p);
}

char *
alloc(n)
register n;
{
	register char *p;

#define round(a,b) ((((a)+(b)-1)/(b))*(b))
	n=round(n,sizeof(char *));
	if (alasta+n < alastr) {
		p = alasta;
		alasta += n;
		return(p);
	}
	if (lasta+n >= lastr) {
		if (sbrk(2000) == (char *)-1) {
			fprintf(stderr, "C Optimizer: out of space\n");
			exit(1);
		}
		lastr += 2000;
	}
	p = lasta;
	lasta += n;
	return(p);
}

clearreg()
{
	register int i;

	for (i=0; i<2*NREG; i++)
		regs[i][0] = '\0';
	conloc[0] = 0;
	ccloc[0] = 0;
}

savereg(ai, as)
char *as;
{
	register char *p, *s, *sp;

	sp = p = regs[ai];
	s = as;
	if (source(s))
		return;
	while (*p++ = *s) {
		if (s[0]=='(' && s[1]=='r' && s[2]<'5') {
			*sp = 0;
			return;
		}
		if (*s++ == ',')
			break;
	}
	*--p = '\0';
}

dest(as, flt)
char *as;
{
	register char *s;
	register int i;

	s = as;
	source(s);
	if ((i = isreg(s)) >= 0)
		regs[i+flt][0] = 0;
/* v7orig:
 *	for (i=0; i<NREG+NREG; i++)
 */
	for (i=flt; i<flt+NREG; i++)
		if (*regs[i]=='*' && equstr(s, regs[i]+1))
			regs[i][0] = 0;
	if (equstr(s, conloc))
		conloc[0] = '\0';
	/*
	 * Type casting between float and int may end up removing
	 * non-redundant moves. This is OK if supporting strict aliasing,
	 * but this compiler is about 40 year too old.
	 */
	while ((i = findrand(s, 0)) >= 0)
		regs[i][0] = 0;
	while ((i = findrand(s, NREG)) >= 0)
		regs[i][0] = 0;
	while (*s) {
		if ((*s=='(' && (*(s+1)!='r' || *(s+2)!='5')) || *s++=='*') {
/* v7.orig:
 *			for (i=0; i<NREG+NREG; i++) {
 */
			for (i=flt; i<flt+NREG; i++) {
				if (regs[i][0] != '$')
					regs[i][0] = 0;
				conloc[0] = 0;
			}
			return;
		}
	}
}

singop(ap)
struct node *ap;
{
	register char *p1, *p2;

	p1 = ap->code;
	p2 = regs[RT1];
	while (*p2++ = *p1++);
	regs[RT2][0] = 0;
}


dualop(ap)
struct node *ap;
{
	register char *p1, *p2;
	register struct node *p;

	p = ap;
	p1 = p->code;
	p2 = regs[RT1];
	while (*p1 && *p1!=',')
		*p2++ = *p1++;
	*p2++ = 0;
	p2 = regs[RT2];
	*p2 = 0;
	if (*p1++ !=',')
		return;
	while (*p1==' ' || *p1=='\t')
		p1++;
	while (*p2++ = *p1++)
		;
}

findrand(as, flt)
char *as;
{
	register int i;
	for (i = flt; i<NREG+flt; i++) {
		if (equstr(regs[i], as))
			return(i);
	}
	return(-1);
}

isreg(as)
char *as;
{
	register char *s;

	s = as;
	if (s[0]=='r' && s[1]>='0' && s[1]<='4' && s[2]==0)
		return(s[1]-'0');
	return(-1);
}

check()
{
	register struct node *p, *lp;
	register count;

	lp = &first;
	count = 0;
	for (p=first.forw; p!=0; p = p->forw) {
		if (++count > 10000)
			abort(0);
		if (p->back != lp)
			abort(1);
		lp = p;
	}
}

source(ap)
char *ap;
{
	register char *p1, *p2;

	p1 = ap;
	p2 = p1;
	if (*p1==0)
		return(0);
	while (*p2++);
	if (*p1=='-' && *(p1+1)=='('
	 || *p1=='*' && *(p1+1)=='-' && *(p1+2)=='('
	 || *(p2-2)=='+') {
		while (*p1 && *p1++!='r');
		if (*p1>='0' && *p1<='4')
			regs[*p1 - '0'][0] = 0;
		return(1);
	}
	return(0);
}

repladdr(p, f, flt)
struct node *p;
{
	register r;
	int r1;
	register char *p1, *p2;
	static char rt1[50], rt2[50];

	if (f)
		r1 = findrand(regs[RT2], flt);
	else
		r1 = -1;
	r = findrand(regs[RT1], flt);
	if (r1 >= NREG)
		r1 -= NREG;
	if (r >= NREG)
		r -= NREG;
	if (r>=0 || r1>=0) {
		p2 = regs[RT1];
		for (p1 = rt1; *p1++ = *p2++;);
		if (regs[RT2][0]) {
			p1 = rt2;
			*p1++ = ',';
			for (p2 = regs[RT2]; *p1++ = *p2++;);
		} else
			rt2[0] = 0;
		if (r>=0) {
			rt1[0] = 'r';
			rt1[1] = r + '0';
			rt1[2] = 0;
			nsaddr++;
			nchange++;
		}
		if (r1>=0) {
			rt2[1] = 'r';
			rt2[2] = r1 + '0';
			rt2[3] = 0;
			nsaddr++;
			nchange++;
		}
		p->code = copy(2, rt1, rt2);
	}
}

movedat()
{
	register struct node *p1, *p2;
	struct node *p3;
	register seg;
	struct node data;
	struct node *datp;

	if (first.forw == 0)
		return;
	if (lastseg != TEXT && lastseg != -1) {
		p1 = (struct node *)alloc(sizeof(first));
		p1->op = lastseg;
		p1->subop = 0;
		p1->code = NULL;
		p1->forw = first.forw;
		p1->back = &first;
		first.forw->back = p1;
		first.forw = p1;
	}
	datp = &data;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op == DATA) {
			p2 = p1->forw;
			while (p2 && p2->op!=TEXT)
				p2 = p2->forw;
			if (p2==0)
				break;
			p3 = p1->back;
			p1->back->forw = p2->forw;
			p2->forw->back = p3;
			p2->forw = 0;
			datp->forw = p1;
			p1->back = datp;
			p1 = p3;
			datp = p2;
		}
	}
	if (data.forw) {
		datp->forw = first.forw;
		first.forw->back = datp;
		data.forw->back = &first;
		first.forw = data.forw;
	}
	seg = lastseg;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op==TEXT||p1->op==DATA||p1->op==BSS) {
			if (p2 = p1->forw) {
				if (p2->op==TEXT||p2->op==DATA||p2->op==BSS)
					p1->op  = p2->op;
			}
			if (p1->op == seg || p1->forw&&p1->forw->op==seg) {
				p1->back->forw = p1->forw;
				p1->forw->back = p1->back;
				p1 = p1->back;
				continue;
			}
			seg = p1->op;
		}
	}
}

redunbr(p)
register struct node *p;
{
	register struct node *p1;
	register char *ap1;
	char *ap2;

	if ((p1 = p->ref) == 0)
		return;
	p1 = nonlab(p1);
	if (p1->op==TST) {
		singop(p1);
		savereg(RT2, "$0");
	} else if (p1->op==CMP)
		dualop(p1);
	else
		return;
	if (p1->forw->op!=CBR)
		return;
	ap1 = findcon(RT1);
	ap2 = findcon(RT2);
	p1 = p1->forw;
	if (compare(p1->subop, ap1, ap2)>0) {
		nredunj++;
		nchange++;
		decref(p->ref);
		p->ref = p1->ref;
		p->labno = p1->labno;
		p->ref->refc++;
	}
}

char *
findcon(i)
{
	register char *p;
	register r;

	p = regs[i];
	if (*p=='$')
		return(p);
	if ((r = isreg(p)) >= 0)
		return(regs[r]);
	if (equstr(p, conloc))
		return(conval);
	return(p);
}

compare(oper, cp1, cp2)
register char *cp1, *cp2;
{
	register unsigned n1, n2;

	if (*cp1++ != '$' || *cp2++ != '$')
		return(-1);
	n1 = 0;
	while (*cp2 >= '0' && *cp2 <= '7') {
		n1 <<= 3;
		n1 += *cp2++ - '0';
	}
	n2 = n1;
	n1 = 0;
	while (*cp1 >= '0' && *cp1 <= '7') {
		n1 <<= 3;
		n1 += *cp1++ - '0';
	}
	if (*cp1=='+')
		cp1++;
	if (*cp2=='+')
		cp2++;
	do {
		if (*cp1++ != *cp2)
			return(-1);
	} while (*cp2++);
	switch(oper) {

	case JEQ:
		return(n1 == n2);
	case JNE:
		return(n1 != n2);
	case JLE:
		return((int)n1 <= (int)n2);
	case JGE:
		return((int)n1 >= (int)n2);
	case JLT:
		return((int)n1 < (int)n2);
	case JGT:
		return((int)n1 > (int)n2);
	case JLO:
		return(n1 < n2);
	case JHI:
		return(n1 > n2);
	case JLOS:
		return(n1 <= n2);
	case JHIS:
		return(n1 >= n2);
	}
	return(-1);
}

setcon(ar1, ar2)
char *ar1, *ar2;
{
	register char *cl, *cv, *p;

	cl = ar2;
	cv = ar1;
	if (*cv != '$')
		return;
	if (!natural(cl))
		return;
	p = conloc;
	while (*p++ = *cl++);
	p = conval;
	while (*p++ = *cv++);
}

equstr(ap1, ap2)
char *ap1, *ap2;
{
	register char *p1, *p2;

	p1 = ap1;
	p2 = ap2;
	do {
		if (*p1++ != *p2)
			return(0);
	} while (*p2++);
	return(1);
}

setcc(ap)
char *ap;
{
	register char *p, *p1;

	p = ap;
	if (!natural(p)) {
		ccloc[0] = 0;
		return;
	}
	p1 = ccloc;
	while (*p1++ = *p++);
}

natural(ap)
char *ap;
{
	register char *p;

	p = ap;
	if (*p=='*' || *p=='(' || *p=='-'&&*(p+1)=='(')
		return(0);
	while (*p++);
	p--;
	if (*--p == '+' || *p ==')' && *--p != '5')
		return(0);
	return(1);
}

xnatural(ap)
	char *ap;
{
	if (natural(ap))
		return(1);
	return(equstr("(sp)", ap));
}

not_sp(ap)
	register char *ap;
{
	char c;

	while (c = *ap++)
		if (c == '(') return(*ap == 's' && ap[1] == 'p');
	return(1);
}
