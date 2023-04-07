#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n2.c	4.2 (2.11BSD) 2020/3/24";
#endif

#include <sgtty.h>
#include <stdlib.h>
#include <stdio.h>

#include "tdef.h"
extern
#include "d.h"
extern
#include "v.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "sdef.h"
#include <setjmp.h>
jmp_buf sjbuf;

/*
troff2.c

output, cleanup
*/

extern struct s *frame, *stk, *nxf;
extern char *enda;


extern int dilev;
extern int eschar;
extern int tlss;
extern int tflg;
extern int ascii;
extern int print;
extern char trtab[];
extern int waitf;
extern char ptname[];
extern int em;
extern int mflg;
extern int nflush;
extern int lgf;
extern int nfo;
extern int donef;
extern int nofeed;
extern int trap;
extern struct sgttyb ttys;
extern int ttysave;
extern int quiet;
extern int ndone;
extern int lead;
extern int ralss;
extern int paper;
extern int gflag;
extern char nextf[];
extern int pipeflg;
extern int ejf;
extern int no_out;
extern int level;
extern int stopmesg;
extern int xxx;
int toolate;
int error;
#ifndef NROFF
extern int acctf;
#endif

extern FILE *fptid;

void
pchar(c)
int c;
{
	register i, j;

	if((i=c) & MOT){pchar1(i); return;}
	switch(j = i & CMASK){
		case 0:
		case IMP:
		case RIGHT:
		case LEFT:
			return;
		case HX:
			j = (tlss>>9) | ((i&~0777)>>3);
			if(i & 040000){
				j &= ~(040000>>3);
				if(j > dip->blss)dip->blss = j;
			}else{
				if(j > dip->alss)dip->alss = j;
				ralss = dip->alss;
			}
			tlss = 0;
			return;
		case LX:
			tlss = i;
			return;
		case PRESC:
			if(dip == &d[0])j = eschar;
		default:
			i = (trtab[j] & BMASK) | (i & ~CMASK);
	}
	pchar1(i);
}

void
pchar1(c)
int c;
{
	register i, j, *k;

	j = (i = c) & CMASK;
	if(dip != &d[0]){
		wbf(i);
		dip->opb = offsb;
		dip->opx = offsx;
		return;
	}
	if(!tflg && !print){
		if(j == '\n')dip->alss = dip->blss = 0;
		return;
	}
	if(no_out || (j == FILLER))return;
#ifndef NROFF
	if(ascii){
		char *s;

		if(i & MOT){
			fputc(' ', fptid);
			return;
		}
		if(j < 0177){
			fputc(i, fptid);
			return;
		}
		switch(j){
			case 0200:
			case 0210:
				s = "-";
				break;
			case 0211:
				s = "fi";
				break;
			case 0212:
				s = "fl";
				break;
			case 0213:
				s = "ff";
				break;
			case 0214:
				s = "ffi";
				break;
			case 0215:
				s = "ffl";
				break;
			default:
				s = NULL;
				for(k=chtab; *++k != j; k++)
					if(*k == 0)return;
				k--;
				fprintf(fptid, "\\(%c%c",
				    *k & BMASK, *k >> BYTE);
		}
		if (s)
			fprintf(fptid, "%s", s);
	}else
#endif
	ptout(i);
}

#if 0
oput(i)
char i;
{
#if 0
	*obufp++ = i;
	if(obufp == (obuf + OBUFSZ + ascii - 1))flusho();
#endif
	toolate |= putc(i, fptid);
}
oputs(i)
char *i;
{
	while(*i != 0)oput(*i++);
}
flusho(){
#if 0
	if(!ascii)*obufp++ = 0;
	if(no_out == 0){
		if (!toolate) {
			toolate++;
#ifdef NROFF
			if(t.bset || t.breset){
				if(ttysave == -1) {
					gtty(1, &ttys);
					ttysave = ttys.sg_flags;
				}
				ttys.sg_flags &= ~t.breset;
				ttys.sg_flags |= t.bset;
				stty(1, &ttys);
			}
			{
			char *p = t.twinit;
			while (*p++)
				;
			write(ptid, t.twinit, p-t.twinit-1);
			}
#endif
		}
		toolate += write(ptid, obuf, obufp-obuf);
	}
	obufp = obuf;
#endif
}
#endif

void
done(x) int x;{
	register i;

	error |= x;
	level = 0;
	lgf = 0;
	if ((i=em)) {
		donef = -1;
		em = 0;
		if(control(i,0))longjmp(sjbuf,1);
	}
	if(!nfo)done3(0);
	mflg = 0;
	dip = &d[0];
	if(woffb)wbt(0);
	if(eblk.pendw)getword(1);
	eblk.pendnf = 0;
	if(donef == 1)done1(0);
	donef = 1;
	ipb = 0;
	frame = stk;
	nxf = frame + 1;
	if(!ejf)tbreak();
	nflush++;
	eject((struct s *)0);
	longjmp(sjbuf,1);
}
done1(x) int x; {
	error |= x;
	if(v.nl){
		trap = 0;
		eject((struct s *)0);
		longjmp(sjbuf,1);
	}
	if(nofeed){
		ptlead();
		done3(0);
	}else{
		if(!gflag)lead += TRAILER;
		done2(0);
	}
}
done2(x) int x; {
	register i;

	ptlead();
#ifndef NROFF
	if(!ascii){
		fprintf(fptid, "%c%c", T_INIT, T_STOP);
		if(!gflag)for(i=8; i>0; i--)fputc(T_PAD, fptid);
		if(stopmesg)warnx("Troff finished.");
	}
#endif
	done3(x);
}
done3(x) int x;{
	error |= x;
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#ifdef NROFF
	twdone();
#endif
	if(quiet){
		ttys.sg_flags |= ECHO;
		stty(0, &ttys);
	}
	if(ascii)mesg(1);
#ifndef NROFF
	report();
#endif
	exit(error);
}
edone(x) int x;{
	frame = stk;
	nxf = frame + 1;
	ipb = 0;
	done(x);
}
#ifndef NROFF
report(){
	struct {int use; int uid;} a;

	if((fptid != stdout) && paper ){
		lseek(acctf,0L,2);
		a.use = paper;
		a.uid = getuid();
		write(acctf,(char *)&a,sizeof(a));
		close(acctf);
	}
}
#endif
#ifdef NROFF

void
casepi(void){
	register i;
	int id[2];

	if(toolate || skip() || !getname() || (pipe(id) == -1) ||
	   ((i=fork()) == -1)){
		warnx("Pipe not created.");
		return;
	}
	if (id[1] != fileno(fptid)) {
		dup2(id[1], fileno(fptid));
		close(id[1]);
	}
	if(i>0){
		close(id[0]);
		toolate++;
		pipeflg++;
		return;
	}
	close(0);
	dup(id[0]);
	close(id[1]);
	execl(nextf,nextf,0);
	err(-4, "Cannot exec: %s", nextf);
}
#endif
