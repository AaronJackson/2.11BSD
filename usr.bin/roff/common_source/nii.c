#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)nii.c	4.2 (2.11BSD) 2020/3/24";
#endif

#include "tdef.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "sdef.h"
#include "d.h"
#include "v.h"
#include <sgtty.h>

int *vlist = (int *)&v;
struct s *frame, *stk, *ejl;
struct s *nxf;

#ifdef NROFF
int pipeflg;
int hflg;
int eqflg;
#endif

#ifndef NROFF
int xpts;
int verm;
int *pslp;
int psflg;
int ppts;
int pfont;
int paper;
int mpts;
int mfont;
int mcase;
int escm;
int cs;
int code;
int ccs;
int bd;
int back;
#endif

int level;
int stdi;
int waitf;
int nofeed;
int quiet;
int stop;
char ibuf[IBUFSZ];
char xbuf[IBUFSZ];
char *ibufp;
char *xbufp;
char *eibuf;
char *xeibuf;
int cbuf[NC];
int *cp;
int nx;
int mflg;
int ch = 0;
int cps;
int ibf;
int ttyod;
struct sgttyb ttys;
int iflg;
char *enda;
int rargc;
char **argp;
char trtab[256];
int lgf;
int copyf;
int ch0;
int cwidth;
int nlflg;
int *ap;
int donef;
int nflush;
int nchar;
int rchar;
int nfo;
int ifile;
int padc;
int raw;
int ifl[NSO];
int ifi;
int flss;
int nonumb;
int trap;
int tflg;
int ejf;
int gflag;
int dilev;
int tlss;
int em;
int ndone;
int lead;
int ralss;
int *argtop;
int nrbits;
int nform;
int macerr;
int wbfi;
int inc[NN];
int fmt[NN];
int evi;
int vflag;
int noscale;
int po1;
int nlist[NTRAP];
int mlist[NTRAP];
int evlist[EVLSZ];
int ev;
int tty;
int sfont;
int sv;
int esc;
int widthp;
int xfont;
int setwdf;
int xbitf;
int over;
int nhyp;
int **hyp;
int *olinep;
int esct;
int ttysave = -1;
int dotT;
int no_out;
int ipb, ipx; /* current read positions */
