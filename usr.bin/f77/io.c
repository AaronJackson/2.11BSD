/* Routines to generate code for I/O statements.
   Some corrections and improvements due to David Wasley, U. C. Berkeley
   Compile with -DKOSHER to force exact conformity with the ANSI std.
*/
/*
 * This module supports the ucb fixed libI77. Since it was done
 * on a Vax, some of the naming conventions were different. These were
 * changed to allow compilation on the 11/70 version. wfj 3/26/80
 * <UNTESTED>
 */
/* Modified to handle encode/decode statements.  
 * Other files modified to support this feature:
 *	lex.c	    "encode" & "decode" added to key[]
 *	tokens	    SENCODE & SDECODE added
 *	gram.io     modified in an obvious way.
 * John Sender, 7/23/80.
 */

#ifdef KOSHER
#define IOSRETURN 1  /* to force ANSI std return on iostat= */
#endif

/* TEMPORARY */
#define TYIOINT TYLONG
#define SZIOINT SZLONG

#include "defs"
#include "string_defs"

LOCAL char ioroutine[XL+1];

LOCAL int ioendlab;
LOCAL int ioerrlab;
LOCAL int iostest;
LOCAL int iosreturn;
LOCAL int jumplab;
LOCAL int skiplab;
LOCAL int ioformatted;
LOCAL expptr ioecdc;	/* encode/decode char cnt */	/* E-D */

#define UNFORMATTED 0
#define FORMATTED 1
#define LISTDIRECTED 2

#define V(z)	ioc[z].iocval

#define IOALL 07777

LOCAL struct ioclist
	{
	char *iocname;
	int iotype;
	expptr iocval;
	} ioc[ ] =
	{
		{ "", 0 },
		{ "unit", IOALL },
		{ "fmt", M(IOREAD) | M(IOWRITE) },
		{ "err", IOALL },
#ifdef KOSHER
		{ "end", M(IOREAD) },
#else
		{ "end", M(IOREAD) | M(IOWRITE) },
#endif
		{ "iostat", IOALL },
		{ "rec", M(IOREAD) | M(IOWRITE) },
		{ "recl", M(IOOPEN) | M(IOINQUIRE) },
		{ "file", M(IOOPEN) | M(IOINQUIRE) },
		{ "status", M(IOOPEN) | M(IOCLOSE) },
		{ "access", M(IOOPEN) | M(IOINQUIRE) },
		{ "form", M(IOOPEN) | M(IOINQUIRE) },
		{ "blank", M(IOOPEN) | M(IOINQUIRE) },
		{ "exist", M(IOINQUIRE) },
		{ "opened", M(IOINQUIRE) },
		{ "number", M(IOINQUIRE) },
		{ "named", M(IOINQUIRE) },
		{ "name", M(IOINQUIRE) },
		{ "sequential", M(IOINQUIRE) },
		{ "direct", M(IOINQUIRE) },
		{ "formatted", M(IOINQUIRE) },
		{ "unformatted", M(IOINQUIRE) },
		{ "nextrec", M(IOINQUIRE) }
	} ;

#define NIOS (sizeof(ioc)/sizeof(struct ioclist) - 1)
#define MAXIO	SZFLAG + 10*SZIOINT + 15*SZADDR

#define IOSUNIT 1
#define IOSFMT 2
#define IOSERR 3
#define IOSEND 4
#define IOSIOSTAT 5
#define IOSREC 6
#define IOSRECL 7
#define IOSFILE 8
#define IOSSTATUS 9
#define IOSACCESS 10
#define IOSFORM 11
#define IOSBLANK 12
#define IOSEXISTS 13
#define IOSOPENED 14
#define IOSNUMBER 15
#define IOSNAMED 16
#define IOSNAME 17
#define IOSSEQUENTIAL 18
#define IOSDIRECT 19
#define IOSFORMATTED 20
#define IOSUNFORMATTED 21
#define IOSNEXTREC 22

#define IOSTP V(IOSIOSTAT)
#define	IOSRW (iostmt==IOREAD || iostmt==IOWRITE)


/* offsets in generated structures */

#define SZFLAG SZIOINT

/* offsets for external READ and WRITE statements */

#define XERR 0
#define XUNIT	SZFLAG
#define XEND	SZFLAG + SZIOINT
#define XFMT	2*SZFLAG + SZIOINT
#define XREC	2*SZFLAG + SZIOINT + SZADDR

/* offsets for internal READ and WRITE statements */

#define XIERR	0
#define XIUNIT	SZFLAG
#define XIEND	SZFLAG + SZADDR
#define XIFMT	2*SZFLAG + SZADDR
#define XIRLEN	2*SZFLAG + 2*SZADDR
#define XIRNUM	2*SZFLAG + 2*SZADDR + SZIOINT
#define XIREC	2*SZFLAG + 2*SZADDR + 2*SZIOINT

/* offsets for OPEN statements */

#define XFNAME	SZFLAG + SZIOINT
#define XFNAMELEN	SZFLAG + SZIOINT + SZADDR
#define XSTATUS	SZFLAG + 2*SZIOINT + SZADDR
#define XACCESS	SZFLAG + 2*SZIOINT + 2*SZADDR
#define XFORMATTED	SZFLAG + 2*SZIOINT + 3*SZADDR
#define XRECLEN	SZFLAG + 2*SZIOINT + 4*SZADDR
#define XBLANK	SZFLAG + 3*SZIOINT + 4*SZADDR

/* offset for CLOSE statement */

#define XCLSTATUS	SZFLAG + SZIOINT

/* offsets for INQUIRE statement */

#define XFILE	SZFLAG + SZIOINT
#define XFILELEN	SZFLAG + SZIOINT + SZADDR
#define XEXISTS	SZFLAG + 2*SZIOINT + SZADDR
#define XOPEN	SZFLAG + 2*SZIOINT + 2*SZADDR
#define XNUMBER	SZFLAG + 2*SZIOINT + 3*SZADDR
#define XNAMED	SZFLAG + 2*SZIOINT + 4*SZADDR
#define XNAME	SZFLAG + 2*SZIOINT + 5*SZADDR
#define XNAMELEN	SZFLAG + 2*SZIOINT + 6*SZADDR
#define XQACCESS	SZFLAG + 3*SZIOINT + 6*SZADDR
#define XQACCLEN	SZFLAG + 3*SZIOINT + 7*SZADDR
#define XSEQ	SZFLAG + 4*SZIOINT + 7*SZADDR
#define XSEQLEN	SZFLAG + 4*SZIOINT + 8*SZADDR
#define XDIRECT	SZFLAG + 5*SZIOINT + 8*SZADDR
#define XDIRLEN	SZFLAG + 5*SZIOINT + 9*SZADDR
#define XFORM	SZFLAG + 6*SZIOINT + 9*SZADDR
#define XFORMLEN	SZFLAG + 6*SZIOINT + 10*SZADDR
#define XFMTED	SZFLAG + 7*SZIOINT + 10*SZADDR
#define XFMTEDLEN	SZFLAG + 7*SZIOINT + 11*SZADDR
#define XUNFMT	SZFLAG + 8*SZIOINT + 11*SZADDR
#define XUNFMTLEN	SZFLAG + 8*SZIOINT + 12*SZADDR
#define XQRECL	SZFLAG + 9*SZIOINT + 12*SZADDR
#define XNEXTREC	SZFLAG + 9*SZIOINT + 13*SZADDR
#define XQBLANK	SZFLAG + 9*SZIOINT + 14*SZADDR
#define XQBLANKLEN	SZFLAG + 9*SZIOINT + 15*SZADDR

fmtstmt(lp)
register struct labelblock *lp;
{
if(lp == NULL)
	{
	error("unlabeled format statement" , 0,0,EXECERR);
	return(-1);
	}
if(lp->labtype == LABUNKNOWN)
	{
	lp->labtype = LABFORMAT;
	lp->labelno = newlabel();
	}
else if(lp->labtype != LABFORMAT)
	{
	error("bad format number", 0,0,EXECERR);
	return(-1);
	}
return(lp->labelno);
}



setfmt(lp)
struct labelblock *lp;
{
ftnint n;
char *s, *lexline();

s = lexline(&n);
preven(ALILONG);
prlabel(asmfile, lp->labelno);
putstr(asmfile, s, n);
flline();
}



startioctl()
{
register int i;

ioecdc = NULL;						/* E-D */
inioctl = YES;
nioctl = 0;
ioformatted = UNFORMATTED;
for(i = 1 ; i<=NIOS ; ++i)
	V(i) = NULL;
}



iosetecdc(e)						/* E-D */
expptr e;						/* E-D */
{							/* E-D */
	ioecdc = e;					/* E-D */
}							/* E-D */



endioctl()
{
int i;
expptr p;
struct labelblock *mklabel();

inioctl = NO;
if(ioblkp == NULL)
	ioblkp = autovar( (MAXIO+SZIOINT-1)/SZIOINT , TYIOINT, NULL);

/* set up for error recovery */

ioerrlab = ioendlab = jumplab = 0;
skiplab = iosreturn = NO;

if(p = V(IOSEND))
	if(ISICON(p))
		ioendlab = mklabel(p->const.ci)->labelno;
	else
		error("bad end= clause",0,0,ERR);

if(p = V(IOSERR))
	if(ISICON(p))
		ioerrlab = mklabel(p->const.ci)->labelno;
	else
		error("bad err= clause",0,0,ERR);

if(IOSTP)
	if(IOSTP->tag!=TADDR || ! ISINT(IOSTP->vtype) )
		{
		error("iostat must be an integer variable",0,0,ERR);
		frexpr(IOSTP);
		IOSTP = NULL;
		}
#ifdef IOSRETURN
	else
		iosreturn = YES;

if(iosreturn && IOSRW && !(ioerrlab && ioendlab) )
	{
	jumplab = newlabel();
	iostest = OPEQ;
	if(ioerrlab || ioendlab) skiplab = YES;
	}
else if(ioerrlab && !ioendlab)

#else
if(ioerrlab && !ioendlab)
#endif
	{
	jumplab = ioerrlab;
	iostest = IOSRW ? OPLE : OPEQ;
	}
else if(!ioerrlab && ioendlab)
	{
	jumplab = ioendlab;
	iostest = OPGE;
	}
else if(ioerrlab && ioendlab)
	{
	iostest = OPEQ;
	if(ioerrlab == ioendlab)
		jumplab = ioerrlab;
	else
		{
		if(!IOSTP) IOSTP = mktemp(TYINT, NULL);
		jumplab = newlabel();
		skiplab = YES;
		}
	}
/*else if(IOSTP)  /* the standard requires this return! */
/*	{
/*	iosreturn = YES;
/*	if(iostmt==IOREAD || iostmt==IOWRITE)
/*		{
/*		jumplab = newlabel();
/*		iostest = OPEQ;
/*		}
/*	}
 */


ioset(TYIOINT, XERR, ICON(ioerrlab!=0 || iosreturn) );

switch(iostmt)
	{
	case IOOPEN:
		dofopen();  break;

	case IOCLOSE:
		dofclose();  break;

	case IOINQUIRE:
		dofinquire();  break;

	case IOBACKSPACE:
		dofmove("f_back"); break;

	case IOREWIND:
		dofmove("f_rew");  break;

	case IOENDFILE:
		dofmove("f_end");  break;

	case IOREAD:
	case IOWRITE:
		startrw();  break;

	default:
		error("impossible iostmt %d", iostmt,0,FATAL1);
	}
for(i = 1 ; i<=NIOS ; ++i)
	if(i!=IOSIOSTAT && V(i)!=NULL)
		frexpr(V(i));
}



iocname()
{
register int i;
int found, mask;

found = 0;
mask = M(iostmt);
for(i = 1 ; i <= NIOS ; ++i)
	if(toklen==strlen(ioc[i].iocname) && eqn(toklen, token, ioc[i].iocname))
		if(ioc[i].iotype & mask)
			return(i);
		else	found = i;
if(found)
	error("invalid control %s for statement", ioc[found].iocname,0,ERR1);
else
	error("unknown iocontrol %s", varstr(toklen, token),0,ERR1 );
return(IOSBAD);
}


ioclause(n, p)
register int n;
register expptr p;
{
struct ioclist *iocp;

++nioctl;
if(n == IOSBAD)
	return;
if(n == IOSPOSITIONAL)
	{
	if(nioctl > IOSFMT)
		{
		error("illegal positional iocontrol",0,0,ERR);
		return;
		}
	n = nioctl;
	}

if(p == NULL)
	{
	if(n == IOSUNIT)
		p = (iostmt==IOREAD ? IOSTDIN : IOSTDOUT);
	else if(n != IOSFMT)
		{
		error("illegal * iocontrol",0,0,ERR);
		return;
		}
	}
if(n == IOSFMT)
	ioformatted = (p==NULL ? LISTDIRECTED : FORMATTED);

iocp = & ioc[n];
if(iocp->iocval == NULL)
	{
	if( !ioecdc )					/* E-D */
		if(n!=IOSFMT && ( n!=IOSUNIT || (p!=NULL && p->vtype!=TYCHAR) ) )
			p = fixtype(p);
	iocp->iocval = p;
}
else
	error("iocontrol %s repeated", iocp->iocname,0,ERR1);
}

/* io list item */

doio(list)
chainp list;
{
struct exprblock *call0();
doiolist(list);
ioroutine[0] = 'e';
putiocall( call0(TYINT, ioroutine) );
}





LOCAL doiolist(p0)
chainp p0;
{
chainp p;
register tagptr q;
register expptr qe;
register struct nameblock *qn;
struct addrblock *tp, *mkscalar();
int range;

for (p = p0 ; p ; p = p->nextp)
	{
	q = p->datap;
	if(q->tag == TIMPLDO)
		{
		exdo(range=newlabel(), q->varnp);
		doiolist(q->datalist);
		enddo(range);
		free(q);
		}
	else	{
		if(q->tag==TPRIM && q->argsp==NULL
		    && q->namep->vdim!=NULL)
			{
			vardcl(qn = q->namep);
			if(qn->vdim->nelt)
				putio( fixtype(cpexpr(qn->vdim->nelt)),
					mkscalar(qn) );
			else
				error("attempt to i/o array of unknown size",0,0,ERR);
			}
		else if(q->tag==TPRIM && q->argsp==NULL &&
		    (qe = memversion(q->namep)) )
			putio(ICON(1),qe);
		else if( (qe = fixtype(cpexpr(q)))->tag==TADDR)
			putio(ICON(1), qe);
		else if(qe->vtype != TYERROR)
			{
			if(iostmt == IOWRITE)
				{
				tp = mktemp(qe->vtype, qe->vleng);
				puteq( cpexpr(tp), qe);
				putio(ICON(1), tp);
				}
			else
/*				error("non-left side in READ list",0,0,ERR);
*/
/*! changed error message PLWard 10/80 */
				error("function in READ list",0,0,ERR);
			}
		frexpr(q);
		}
	}
frchain( &p0 );
}





LOCAL putio(nelt, addr)
expptr nelt;
register expptr addr;
{
int type;
register struct exprblock *q;
struct exprblock *call2(), *call3();

type = addr->vtype;
if(ioformatted!=LISTDIRECTED && ISCOMPLEX(type) )
	{
	nelt = mkexpr(OPSTAR, ICON(2), nelt);
	type -= (TYCOMPLEX-TYREAL);
	}

/* pass a length with every item.  for noncharacter data, fake one */
if(type != TYCHAR)
	{
	if( ISCONST(addr) )
		addr = putconst(addr);
	addr->vtype = TYCHAR;
	addr->vleng = ICON( typesize[type] );
	}

nelt = fixtype( mkconv(TYLENG,nelt) );
if(ioformatted == LISTDIRECTED)
	q = call3(TYINT, "do_lio", mkconv(TYLONG, ICON(type)), nelt, addr);
else
	q = call2(TYINT, (ioformatted==FORMATTED ? "do_fio" : "do_uio"),
		nelt, addr);
putiocall(q);
}




endio()
{
if(skiplab)
	{
	putlabel(jumplab);
	if(ioendlab) putif( mkexpr(OPGE, cpexpr(IOSTP), ICON(0)), ioendlab);
	if(ioerrlab) putif( mkexpr(OPLE, cpexpr(IOSTP), ICON(0)), ioerrlab);
	}
else if(iosreturn && jumplab)
	putlabel(jumplab);
if(IOSTP)
	frexpr(IOSTP);
}



LOCAL putiocall(q)
register struct exprblock *q;
{
if(IOSTP)
	{
	q->vtype = TYINT;
	q = fixexpr( mkexpr(OPASSIGN, cpexpr(IOSTP), q));
	}

if(jumplab)
	putif( mkexpr(iostest, q, ICON(0) ), jumplab);
else
	putexpr(q);
}


startrw()
{
register expptr p;
register struct nameblock *np;
register struct addrblock *unitp, *nump;
struct constblock *mkaddcon();
int k, fmtoff;
int intfile, sequential;

intfile = NO;
if(p = V(IOSUNIT))
	{
	if( ISINT(p->vtype) )
		ioset(TYIOINT, XUNIT, cpexpr(p) );
	else if( ioecdc || p->vtype == TYCHAR)		/* E-D */
		{
		intfile = YES;
		if(p->tag==TPRIM && p->argsp==NULL &&
		    (np = p->namep)->vdim!=NULL)
			{
			vardcl(np);
			if( ioecdc )			/* E-D */
				nump = ICON(1);		/* E-D */
			else if(np->vdim->nelt)		/* E-D */
				nump = cpexpr(np->vdim->nelt);
			else
				{
				error("attempt to use internal unit array of unknown size",0,0,ERR);
				nump = ICON(1);
				}
			unitp = mkscalar(np);
			}
		else	{
			nump = ICON(1);
			unitp = fixtype(cpexpr(p));
			}
		if( ioecdc )				/* E-D */
			{				/* E-D */
			frexpr( unitp->vleng );		/* E-D */
			unitp->vleng = ioecdc;		/* E-D */
			}				/* E-D */
		ioset(TYIOINT, XIRNUM, nump);
		ioset(TYIOINT, XIRLEN, cpexpr(unitp->vleng) );
		ioset(TYADDR, XIUNIT, addrof(unitp) );
		}
	}
else
	error("bad unit specifier",0,0,ERR);

sequential = YES;
if(p = V(IOSREC))
	if( ISINT(p->vtype) )
		{
		ioset(TYIOINT, (intfile ? XIREC : XREC), cpexpr(p) );
		sequential = NO;
		}
	else
		error("bad REC= clause",0,0,ERR);

ioset(TYIOINT, (intfile ? XIEND : XEND), ICON(ioendlab!=0 || iosreturn) );

fmtoff = (intfile ? XIFMT : XFMT);

if(p = V(IOSFMT))
	{
	if(p->tag==TPRIM && p->argsp==NULL)
		{
		vardcl(np = p->namep);
		if(np->vdim)
			{
			ioset(TYADDR, fmtoff, addrof(mkscalar(np)) );
			goto endfmt;
			}
		if( ISINT(np->vtype) )
			{
			ioset(TYADDR, fmtoff, p);
			goto endfmt;
			}
		}
	p = V(IOSFMT) = fixtype(p);
	if(p->vtype == TYCHAR)
		ioset(TYADDR, fmtoff, addrof(cpexpr(p)) );
	else if( ISICON(p) )
		{
		if( (k = fmtstmt( mklabel(p->const.ci) )) > 0 )
			ioset(TYADDR, fmtoff, mkaddcon(k) );
		else
			ioformatted = UNFORMATTED;
		}
	else	{
		error("bad format descriptor",0,0,ERR);
		ioformatted = UNFORMATTED;
		}
	}
else
	ioset(TYADDR, fmtoff, ICON(0) );

endfmt:
	if(intfile && ioformatted==UNFORMATTED)
		error("unformatted internal I/O not allowed",0,0,ERR);
	if(!sequential && ioformatted==LISTDIRECTED)
		error("direct list-directed I/O not allowed",0,0,ERR);

ioroutine[0] = 's';
ioroutine[1] = '_';
ioroutine[2] = (iostmt==IOREAD ? 'r' : 'w');
ioroutine[3] = (sequential ? 's' : 'd');
ioroutine[4] = "ufl" [ioformatted];
ioroutine[5] = (intfile ? 'i' : 'e');
ioroutine[6] = '\0';
putiocall( call1(TYINT, ioroutine, cpexpr(ioblkp) ));
}



LOCAL dofopen()
{
register expptr p;

if( (p = V(IOSUNIT)) && ISINT(p->vtype) )
	ioset(TYIOINT, XUNIT, cpexpr(p) );
else
	error("bad unit in open",0,0,ERR);
if( (p = V(IOSFILE)) )
	if(p->vtype == TYCHAR)
		ioset(TYIOINT, XFNAMELEN, cpexpr(p->vleng) );
	else
		error("bad file in open",0,0,ERR);

iosetc(XFNAME, p);

if(p = V(IOSRECL))
	if( ISINT(p->vtype) )
		ioset(TYIOINT, XRECLEN, cpexpr(p) );
	else
		error("bad recl",0,0,ERR);
else
	ioset(TYIOINT, XRECLEN, ICON(0) );

iosetc(XSTATUS, V(IOSSTATUS));
iosetc(XACCESS, V(IOSACCESS));
iosetc(XFORMATTED, V(IOSFORM));
iosetc(XBLANK, V(IOSBLANK));

putiocall( call1(TYINT, "f_open", cpexpr(ioblkp) ));
}


LOCAL dofclose()
{
register expptr p;

if( (p = V(IOSUNIT)) && ISINT(p->vtype) )
	{
	ioset(TYIOINT, XUNIT, cpexpr(p) );
	iosetc(XCLSTATUS, V(IOSSTATUS));
	putiocall( call1(TYINT, "f_clos", cpexpr(ioblkp)) );
	}
else
	error("bad unit in close statement",0,0,ERR);
}


LOCAL dofinquire()
{
register expptr p;
if(p = V(IOSUNIT))
	{
	if( V(IOSFILE) )
		error("inquire by unit or by file, not both",0,0,ERR);
	ioset(TYIOINT, XUNIT, cpexpr(p) );
	}
else if( ! V(IOSFILE) )
	error("must inquire by unit or by file",0,0,ERR);
iosetlc(IOSFILE, XFILE, XFILELEN);
iosetip(IOSEXISTS, XEXISTS);
iosetip(IOSOPENED, XOPEN);
iosetip(IOSNUMBER, XNUMBER);
iosetip(IOSNAMED, XNAMED);
iosetlc(IOSNAME, XNAME, XNAMELEN);
iosetlc(IOSACCESS, XQACCESS, XQACCLEN);
iosetlc(IOSSEQUENTIAL, XSEQ, XSEQLEN);
iosetlc(IOSDIRECT, XDIRECT, XDIRLEN);
iosetlc(IOSFORM, XFORM, XFORMLEN);
iosetlc(IOSFORMATTED, XFMTED, XFMTEDLEN);
iosetlc(IOSUNFORMATTED, XUNFMT, XUNFMTLEN);
iosetip(IOSRECL, XQRECL);
iosetip(IOSNEXTREC, XNEXTREC);
iosetlc(IOSBLANK, XQBLANK, XQBLANKLEN);

putiocall( call1(TYINT,  "f_inqu", cpexpr(ioblkp) ));
}



LOCAL dofmove(subname)
char *subname;
{
register expptr p;

if( (p = V(IOSUNIT)) && ISINT(p->vtype) )
	{
	ioset(TYIOINT, XUNIT, cpexpr(p) );
	putiocall( call1(TYINT, subname, cpexpr(ioblkp) ));
	}
else
	error("bad unit in I/O motion statement",0,0,ERR);
}



LOCAL ioset(type, offset, p)
int type, offset;
expptr p;
{
register struct addrblock *q;

q = cpexpr(ioblkp);
q->vtype = type;
q->memoffset = fixtype( mkexpr(OPPLUS, q->memoffset, ICON(offset)) );
puteq(q, p);
}




LOCAL iosetc(offset, p)
int offset;
register expptr p;
{
if(p == NULL)
	ioset(TYADDR, offset, ICON(0) );
else if(p->vtype == TYCHAR)
	ioset(TYADDR, offset, addrof(cpexpr(p) ));
else
	error("non-character control clause",0,0,ERR);
}



LOCAL iosetip(i, offset)
int i, offset;
{
register expptr p;

if(p = V(i))
	if(p->tag==TADDR &&
	    ONEOF(p->vtype, M(TYLONG)|M(TYLOGICAL)) )
		ioset(TYADDR, offset, addrof(cpexpr(p)) );
	else
		error("impossible inquire parameter %s", ioc[i].iocname,0,ERR1);
else
	ioset(TYADDR, offset, ICON(0) );
}



LOCAL iosetlc(i, offp, offl)
int i, offp, offl;
{
register expptr p;
if( (p = V(i)) && p->vtype==TYCHAR)
	ioset(TYIOINT, offl, cpexpr(p->vleng) );
iosetc(offp, p);
}
