#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n10.c	4.3 (2.11BSD) 2020/3/24";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sgtty.h>

#include "tdef.h"
extern
#include "d.h"
extern
#include "v.h"
#include "tw.h"
/*
nroff10.c

Device interfaces
*/

extern int xfont;
extern int esc;
extern int lead;
extern int oline[];
extern int *olinep;
extern int ulfont;
extern int esct;
extern int ttysave;
extern struct sgttyb ttys;
extern char termtab[];
extern int waitf;
extern int pipeflg;
extern int eqflg;
extern int hflg;
extern int ascii;
extern int xxx;
int dtab;
int bdmode;
int plotmode;

extern FILE *fptid;
#define ISWS(x) ((x) == ' ' || (x) == '\t')

#define INCH	240
/*
 * Default output device, suitable for VT100 or similar
 * See description in README in nroff_term
 */
struct tw t = {
	/* printer control codes */
	0, 0, INCH/10, INCH/6, INCH/6, INCH/10, INCH/10, INCH/12, INCH/10,
	"", "", "\n", "", "", "", "", "", "", "", "", "", "", "", 

	{ /* character codetab, first ascii */
	"\001 ", "\001!", "\001\"", "\001#", "\001$", "\001%", "\001&", "\001'",
	"\001(", "\001)", "\001*", "\001+", "\001,", "\001-", "\001.", "\001/",
	"\2010", "\2011", "\2012", "\2013", "\2014", "\2015", "\2016", "\2017",
	"\2018", "\2019", "\001:", "\001;", "\001<", "\001=", "\001>", "\001?",

	"\001@", "\201A", "\201B", "\201C", "\201D", "\201E", "\201F", "\201G", 
	"\201H", "\201I", "\201J", "\201K", "\201L", "\201M", "\201N", "\201O",
	"\201P", "\201Q", "\201R", "\201S", "\201T", "\201U", "\201V", "\201W",
	"\201X", "\201Y", "\201Z", "\001[", "\001\\", "\001]", "\001^", "\001_",

	"\001`", "\201a", "\201b", "\201c", "\201d", "\201e", "\201f", "\201g",
	"\201h", "\201i", "\201j", "\201k", "\201l", "\201m", "\201n", "\201o",
	"\201p", "\201q", "\201r", "\201s", "\201t", "\201u", "\201v", "\201w",
	"\201x", "\201y", "\201z", "\001{", "\001|", "\001}", "\001~", "\000",

	/* following special chars */
	"\1-", "\1o", "\2[]", "\2--", "\001_", "\0031/4", "\0031/2", "\0033/4",
	"\1-", "\202fi", "\202fl", "\202ff", "\203ffi", "\203ffl", "\1o", "\1-",
	"\001S", "\001'", "\001'", "\001`", "\001_", "\001/", "\000", "\000",
	"\201a", "\201B", "\201y", "\201d", "\201e", "\201C", "\201n", "\2010",

	"\201i", "\201k", "\201\\", "\201u", "\201v", "\201E", "\201o", "\201n",
	"\201p", "\201o", "\201t", "\201u", "\201o", "\201x", "\201u", "\201w",
	"\201G", "\201D", "\201O", "\202/\\", "\201X", "\202TT", "\201S", "\0",
	"\201Y", "\202O|", "\202U|", "\202O_", "\002v/", "\0", "\1~", "\2>=",

	"\002<=", "\002==", "\01-", "\02~=", "\01~", "\02!=", "\02->", "\02<-",
	"\001^", "\001v", "\01=", "\01x", "\03-:-", "\02+-", "\01U", "\03(^)",
	"\002(=", "\002=)", "\003(=_", "\003=_)",
	"\002oo", "\002o`", "\003\\~/", "\001~",
	"\001I", "\002oc", "\002{}", "\001E",
	"\001+", "\003(R)", "\003(C)", "\001|",

	"\001c", "\003=|=", "\002=>", "\002<=",
	"\001*", "\000\0", "\001|", "\001O",
	"\002,-", "\002`-", "\002-.", "\002-'",
	"\001{", "\001}", "\001|", "\002|_",
	"\002_|", "\002|~", "\002~|",
	}
};

/*
 * Convert string to binary.
 * loops over the string twice, first to find out needed space, then to save.
 * leaves on byte space first in string for width.
 */
static char *
sdup(register char *s)
{
	static char zc[2];
	register int n;
	register char *rv;
	char *os = s, *srv;
	int echar, asz, run1;

	run1 = 1;

loop2:	s = os;
	if (*s == '\"') {
		echar = *s++;
	} else
		echar = 0;
	while (*s && !ISWS(*s) && *s != '\n' && *s != echar) {
		if (*s++ == '\\') {
			switch (*s) {
			case 'a': n = '\a'; break;
			case 'b': n = '\b'; break;
			case 'f': n = '\f'; break;
			case 'n': n = '\n'; break;
			case 'r': n = '\r'; break;
			case 't': n = '\t'; break;
			case 'v': n = '\v'; break;
			case '\"': n = '\"'; break;
			case '\'': n = '\''; break;
			case '\?': n = '\?'; break;
			case '\\': n = '\\'; break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				n = (*s++ - '0');
				if (*s >= '0' && *s <= '7') {
					n = (n << 3) + (*s++ - '0');
					if (*s >= '0' && *s <= '7')
						n = (n << 3) + (*s++ - '0');
				}
				--s;
				break;
			default:
				n = *s;
				break;
			}
		} else
			n = *--s;
		if (run1)
			run1++;
		else
			*rv++ = n;
		s++;
	}
	if (echar && *s != echar)
		errx(1, "bad control %s", os);
	if (run1) {
		static char *csave;
		static int num;
		if (num < ++run1)
			csave = malloc(num = 256);
		srv = rv = csave;
		csave += run1;
		num -= run1;
		rv++; /* space for width */
		run1 = 0;
		goto loop2;
	}
	*rv = 0;

	return srv;
}

/*
 * Table containing printer properties.  */
static char *twd[] = {
	"bset", "breset", "Hor", "Vert", "Newline",
	"Char", "Em", "Halfline", "Adj",
	"twinit", "twrest", "twnl", "hlr", "hlf", "flr", "bdon",
	"bdoff", "ploton", "plotoff", "up", "down", "right", "left"
};
#define NINTS	9	/* first 9 entries are ints */
#define MAXTWD	(sizeof(twd)/sizeof(twd[0]))	/* total # entries */

/*
 * Read config for output device (if given).
 * Otherwise expect a plain ascii output (for reading man pages).
 */
void
ptinit(void)
{
	FILE *fp;
	register int i;
	register char **p, *r;
	int *ip, n;
	char buf[BUFSIZ], buf2[BUFSIZ];

	if (devtab) {
		/* clear special characters */
		for (i = 128-32; i < 256-32; i++)
			t.codetab[i] = "\0";
		/*
		 * devtab search order; first try devtab directly, 
		 * then search in _PATH_TERMTAB.
		 */
		findfile(FTERMTAB, buf, devtab);
		if ((fp = fopen(buf, "r")) == NULL)
			errx(1, "Cannot open %s", devtab);
		setbuf(fp, buf);

		ip = (int *)&t;
		p = (char **)&t.twinit;
		while (fgets(buf2, BUFSIZ, fp) != NULL) {
			if (buf2[0] == '\n' || buf2[0] == '#')
				continue;
			for (r = buf2; *r && !ISWS(*r); r++)
				;
			*r++ = 0;
			while (ISWS(*r))
				r++;
			for (i = 0; i < MAXTWD; i++)
				if (strcmp(buf2, twd[i]) == 0)
					break;
			if (i < NINTS) {
				ip[i] = 0;
				while (*r >= '0' && *r <= '9')
					ip[i] = ip[i] * 10 + *r++ - '0';
			} else if (i < MAXTWD) {
				p[i-NINTS] = sdup(r) + 1;
			} else if (buf2[2] == 0) {
				i = PAIR(buf2[0],buf2[1]);
				for (ip = chtab; *ip != i; ip++)
					if (*(ip++) == 0)
						goto bad;
				i = *(++ip);
				if (*r < '0' || *r > '9')
					goto bad;
				n = *r++ - '0';
				while (ISWS(*r))
					r++;
				t.codetab[i - 32] = sdup(r);
				*t.codetab[i - 32] = n | 0200;
			} else
bad:				errx(1, "unknown directive %s", buf2);
		}
		fclose(fp);
	}

	eblk.sps = EM;
	eblk.ics = EM*2;
	dtab = 8 * t.Em;
	for(i=0; i<16; i++)eblk.tabtab[i] = dtab * (i+1);
	if(eqflg)t.Adj = t.Hor;
}

twdone(){
	fprintf(fptid, "%s", t.twrest);
	if(pipeflg){
		fclose(fptid);
		wait(&waitf);
	}
	if(ttysave != -1) {
		ttys.sg_flags = ttysave;
		stty(1, &ttys);
	}
}

void
ptout(i)
int i;
{
	*olinep++ = i;
	if(olinep >= &oline[LNSIZE])olinep--;
	if((i&CMASK) != '\n')return;
	olinep--;
	lead += dip->blss + eblk.lss - t.Newline;
	dip->blss = 0;
	esct = esc = 0;
	if(olinep>oline){
		move();
		ptout1();
		fprintf(fptid, "%s", t.twnl);
	}else{
		lead += t.Newline;
		move();
	}
	lead += dip->alss;
	dip->alss = 0;
	olinep = oline;
}
ptout1()
{
	register i, k;
	register char *codep;
	extern char *plot();
	int *q, w, j, phyw;

	for(q=oline; q<olinep; q++){
	if((i = *q) & MOT){
		j = i & ~MOTV;
		if(i & NMOT)j = -j;
		if(i & VMOT)lead += j;
		else esc += j;
		continue;
	}
	if((k = (i & CMASK)) <= 040){
		switch(k){
			case ' ': /*space*/
				esc += t.Char;
				break;
		}
		continue;
	}
	codep = t.codetab[k-32];
	w = t.Char * (*codep++ & 0177);
	phyw = w;
	if(i&ZBIT)w = 0;
	if(*codep && (esc || lead))move();
	esct += w;
	if(i&074000)xfont = (i>>9) & 03;
	if(!bdmode && (xfont == 2)){
		if (*t.bdon)
			fprintf(fptid, "%s", t.bdon);
		bdmode++;
	}
	if(bdmode && (xfont != 2)){
		if (*t.bdoff)
			fprintf(fptid, "%s", t.bdoff);
		bdmode = 0;
	}
	if(xfont == ulfont){
		for(k=w/t.Char;k>0;k--)fputc('_', fptid);
		for(k=w/t.Char;k>0;k--)fputc('\b', fptid);
	}
	while(*codep != 0){
		if(*codep & 0200){
			codep = plot(codep);
			fprintf(fptid, "%s ", t.plotoff);
		}else{
			if(plotmode)fprintf(fptid, "%s", t.plotoff);
			/* if no special bold on/off, print the char twice */
			if (bdmode && *t.bdon == 0)
				fprintf(fptid, "%c\b", *codep);
			putc(*codep++, fptid);
#if 0
			*obufp++ = *codep++;
			if(obufp == (obuf + OBUFSZ + ascii - 1))flusho();
#endif
/*			oput(*codep++);*/
		}
	}
	if(!w)for(k=phyw/t.Char;k>0;k--)fputc('\b', fptid);
	}
}
char *plot(x)
char *x;
{
	register int i;
	register char *j, *k;

	if(!plotmode)fprintf(fptid, "%s", t.ploton);
	k = x;
	if((*k & 0377) == 0200)k++;
	for(; *k; k++){
		if(*k & 0200){
			if(*k & 0100){
				if(*k & 040)j = t.up; else j = t.down;
			}else{
				if(*k & 040)j = t.left; else j = t.right;
			}
			if(!(i = *k & 037))return(++k);
			while(i--)fprintf(fptid, "%s", j);
		}else fputc(*k, fptid);
	}
	return(k);
}
move(){
	register k;
	register char *i, *j;
	char *p, *q;
	int iesct, dt;

	iesct = esct;
	if(esct += esc)i = "\0"; else i = "\n\0";
	j = t.hlf;
	p = t.right;
	q = t.down;
	if(lead){
		if(lead < 0){
			lead = -lead;
			i = t.flr;
		/*	if(!esct)i = t.flr; else i = "\0";*/
			j = t.hlr;
			q = t.up;
		}
		if(*i & 0377){
			k = lead/t.Newline;
			lead = lead%t.Newline;
			while(k--)fprintf(fptid, "%s", i);
		}
		if(*j & 0377){
			k = lead/t.Halfline;
			lead = lead%t.Halfline;
			while(k--)fprintf(fptid, "%s", j);
		}
		else { /* no half-line forward, not at line begining */
			k = lead/t.Newline;
			lead = lead%t.Newline;
			if (k>0) esc=esct;
			i = "\n";
			while (k--) fprintf(fptid, "%s", i);
		}
	}
	if(esc){
		if(esc < 0){
			esc = -esc;
			j = "\b";
			p = t.left;
		}else{
			j = " ";
			if(hflg)while((dt = dtab - (iesct%dtab)) <= esc){
				if(dt%t.Em || dt==t.Em)break;
				fputc(TAB, fptid);
				esc -= dt;
				iesct += dt;
			}
		}
		k = esc/t.Em;
		esc = esc%t.Em;
		while(k--)fprintf(fptid, "%s", j);
	}
	if((*t.ploton & 0377) && (esc || lead)){
		if(!plotmode)fprintf(fptid, "%s", t.ploton);
		esc /= t.Hor;
		lead /= t.Vert;
		while(esc--)fprintf(fptid, "%s", p);
		while(lead--)fprintf(fptid, "%s", q);
		fprintf(fptid, "%s", t.plotoff);
	}
	esc = lead = 0;
}
ptlead(){move();}
dostop(){
	char junk;

	fflush(fptid);
	read(2,&junk,1);
}
