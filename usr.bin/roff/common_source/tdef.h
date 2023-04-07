/*	tdef.h	4.9	(2.11BSD) 2020/3/24 */

#include <sys/param.h>
#undef CMASK			/* XXX */
#define MAXPTR (-1)		/* max value of any pointer variable */
#ifdef NROFF	/*NROFF*/
#define EM t.Em
#define HOR t.Hor
#define VERT t.Vert
#define INCH 240	/*increments per inch*/
#define SPS INCH/10	/*space size*/
#define SS INCH/10	/* " */
#define TRAILER 0
#define UNPAD 0227
#define PO 0 /*page offset*/
#define ASCII 1
#define LG 0
#define DTAB 0	/*set at 8 Ems at init time*/
#define ICS 2*SPS
#define TEMP 256	/*65K*/
#endif
#ifndef NROFF	/*TROFF*/
#define INCH 432	/*troff resolution*/
#define SPS 20	/*space size at 10pt; 1/3 Em*/
#define SS 12	/*space size in 36ths of an em*/
#define TRAILER 6048	/*144*14*3 = 14 inches*/
#define UNPAD 027
#define PO 416 /*page offset 26/27ths inch*/
#define HOR 1
#define VERT 3
#define EM (6*(eblk.pts&077))
#define ASCII 0
#define LG 1
#define DTAB (INCH/2)
#define ICS 3*SPS
#define TEMP 512	/*128K*/
#endif

#include <signal.h>
#define NARSP 0177	/*narrow space*/
#define HNSP 0226	/*half narrow space*/
#define PS 10	/*default point size*/
#define FT 0	/*default font position*/
#define LL 65*INCH/10	/*line length; 39picas=6.5in*/
#define VS INCH/6	/*vert space; 12points*/
#define	NS 64
#define NNAMES 14 /*predefined reg names*/
#define NIF 15	/*if-else nesting*/
#define NTM 256	/*tm buffer*/
#define NEV 3	/*environments*/
#define	EVBASE	((long)NBLIST*BLK*sizeof(int))
#define EVLSZ 10	/*size of ev stack*/

#define NHYP 10	/*max hyphens per word*/
#define NHEX 128	/*byte size of exception word list*/
#define NTAB 35	/*tab stops*/
#define NSO 5	/*"so" depth*/
#define NDI 5	/*number of diversions*/
#define DBL 0100000	/*double size indicator*/
#define MOT 0100000	/*motion character indicator*/
#define MOTV 0160000	/*clear for motion part*/
#define VMOT 0040000	/*vert motion bit*/
#define NMOT 0020000	/* negative motion indicator*/
#define MMASK 0100000	/*macro mask indicator*/
#define CMASK 0100377
#define ZBIT 0400	/*zero width char*/
#define BMASK 0377
#define BYTE 8
#define IMP 004	/*impossible char*/
#define FILLER 037
#define PRESC 026
#define HX 0376	/*High-order part of xlss*/
#define LX 0375	/*low-order part of xlss*/
#define CONT 025
#define COLON 013
#define XPAR 030
#define ESC 033
#define FLSS 031
#define RPT 014
#define JREG 0374
#define NTRAP 20	/*number of traps*/
#define NPN 20	/*numbers in "-o"*/
#define T_PAD 0101	/*cat padding*/
#define T_INIT 0100
#define T_IESC 16 /*initial offset*/
#define T_STOP 0111
#define NPP 10	/*pads per field*/
#define NC 256	/*cbuf size words*/
#define NOV 10	/*number of overstrike chars*/
#define TDELIM 032
#define LEFT 035
#define RIGHT 036
#define LEADER 001
#define TAB 011
#define TMASK  037777
#define RTAB 0100000
#define CTAB 0040000
#define OHC 024

#define PAIR(A,B) (A|(B<<BYTE))

/* Tunable sizes */
#define MAXSTKSZ	1024	/* max size of macro argument stack (ints) */
#define	DELTA		128	/* stack increments bytes */
#define OBUFSZ		1024	/* output buffer bytes */
#define IBUFSZ		1024	/* input buffer bytes */
#define EBUFSZ		128	/* error buffer bytes (for messages) */
#define CONHSZ		127	/* macros/requests hash table size */
#define LNSIZE		680	/* line buffer size */
#define WDSIZE		170	/* word buffer size */
#define FBUFSZ		64	/* field buf size words */
#define NN		600	/* number registers */

#define NBLIST		1024	/* # of blocks in macro file */
#define BLK		128	/* alloc block words */
#define BLKBITS		7	/* for BLK=128 */
#define BLKCSZ		8	/* # of blocks cached, must be power of 2 */
#define	MACHUNK		128	/* macro allocation chunk */

/* Environment struct */
struct eblk {
	int block, ics, ic, icf, chbits, spbits, nmbits, apts,
	apts1, pts, pts1, font, font1, sps, spacesz, lss,
	lss1, ls, ls1, ll, ll1, lt, lt1, ad,
	nms, ndf, fi, cc, c2, ohc, tdelim, hyf,
	hyoff, un1, tabc, dotc, adsp, adrem, lastl, nel,
	admod, *wordp, spflg, *linep, *wdend, *wdstart, wne, ne,
	nc, nb, lnmod, nwd, nn, ni, ul, cu,
	ce, in, in1, un, wch, pendt, *pendw, pendnf,
	spread, it, itmac, lnsize,
	*hyptr[NHYP],
	tabtab[NTAB],
	line[LNSIZE],
	word[WDSIZE];
};

/* description of a request or macro */
struct constr {
	struct constr *next;
	union {
		void (*f)(void);
		int mx;
	} x;
	int rq;		/* encoded */
	int flags;
};
#define	CONMAC	1	/* contains a macro, not request */
#define	CONLRQ	2	/* contains a long (string) request */
#define	ISMAC(x)	((x)->flags & CONMAC)
#define	ISLRQ(x)	((x)->flags & CONLRQ)

/* max request uses one extra buffer */
#define	MAXRQSZ		(sizeof(struct constr) + DFLREQSZ)

/* Type of file to search for */
#define FTERMTAB	1
#define FTMAC		2
#define FTFONT		3


/* external vars */
extern struct eblk eblk;
extern struct constr constr[];
extern int ipb, ipx;
extern int nextbb, nextbx;
extern int offsb, offsx;
extern int woffb;
extern char *fontdir;
extern char *devtab;
extern int chtab[];

struct s;

/* prototypes */
int pushi(int);
int alloc(void);
void ffree(int);
int rbf0(int, int);
void wbfl(void);
int incb(int);
int hseg(int (*f)(),int pb, int px);
void inithsh(void);
int rbf(int);
struct constr *findmn(int);
void mesg(int);
void getpn(char *a), setline(void), setov(void), setbra(void), ptout(int);
void flushi(void), eject(struct s *), hyphen(int *), setvline(void);
void pchar(int), pchar1(int c), text(void), newline(int);
void wbf(int), setn(void), setfont(int), setps(void), setwd(void);
int control(int, int), getch(void);
char *findfile(int, char *, char *);
char *setbrk(int);

/* request prototypes */
void caseds(), caseas(), casesp(void), caseft(), caseps(), casevs(),
casenr(), caseif(), casepo(), casetl(), casetm(void), casebp(), casech(),
casepn(), tbreak(), caseti(), casene(), casenf(), casece(), casefi(),
casein(), casell(), casens(), casemk(), casert(), caseam(),
casede(), casedi(), caseda(), casewh(), casedt(), caseit(), caserm(),
casern(), casead(), casers(), casena(), casepl(), caseta(), casetr(),
caseul(), caselt(), casenx(), caseso(), caseig(), casetc(), casefc(),
caseec(), caseeo(), caselc(), caseev(), caserd(), caseab(), casefl(),
done(), casess(), casefp(), casecs(), casebd(), caselg(), casehc(),
casehy(), casenh(), casenm(), casenn(), casesv(), caseos(), casels(),
casecc(), casec2(), caseem(), caseaf(), casehw(), casemc(), casepm(),
casecu(), casepi(), caserr(), caseuf(), caseie(), caseel(), casepc(),
caseht();
#ifndef NROFF
void casefz();
#endif
void casecf();

/* some have args, have separate wrappers */
void casespx(int);
