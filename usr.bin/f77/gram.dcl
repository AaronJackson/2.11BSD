spec:	  dcl
	| common
	| external
	| intrinsic
	| equivalence
	| data
	| implicit
	| SSAVE
		{ saveall = YES; }
	| SSAVE savelist
	| SFORMAT
		{ fmtstmt(thislabel); setfmt(thislabel); }
	| SPARAM in_dcl SLPAR paramlist SRPAR
	;

dcl:	  type name in_dcl lengspec dims
		{ settype($2, $1, $4);
		  if(ndim>0) setbound($2,ndim,dims);
		}
	| dcl SCOMMA name lengspec dims
		{ settype($3, $1, $4);
		  if(ndim>0) setbound($3,ndim,dims);
		}
	;

type:	  typespec lengspec
		{ varleng = $2; }
	;

typespec:  typename
		{ varleng = ($1<0 || $1==TYLONG ? 0 : typesize[$1]); }
	;

typename:    SINTEGER	{ $$ = TYLONG; }
	| SREAL		{ $$ = TYREAL; }
	| SCOMPLEX	{ $$ = TYCOMPLEX; }
	| SDOUBLE	{ $$ = TYDREAL; }
	| SDCOMPLEX	{ $$ = TYDCOMPLEX; }
	| SLOGICAL	{ $$ = TYLOGICAL; }
	| SCHARACTER	{ $$ = TYCHAR; }
	| SUNDEFINED	{ $$ = TYUNKNOWN; }
	| SDIMENSION	{ $$ = TYUNKNOWN; }
	| SAUTOMATIC	{ $$ = - STGAUTO; }
	| SSTATIC	{ $$ = - STGBSS; }
	;

lengspec:
		{ $$ = varleng; }
	| SSTAR intonlyon expr intonlyoff
		{
		  if( ! ISICON($3) )
			{
			$$ = 0;
			error("length must be an integer constant", 0, 0, DCLERR);
			}
		  else $$ = $3->const.ci;
		}
	| SSTAR intonlyon SLPAR SSTAR SRPAR intonlyoff
		{ $$ = 0; }
	;

common:	  SCOMMON in_dcl var
		{ incomm( $$ = comblock(0, 0) , $3 ); }
	| SCOMMON in_dcl comblock var
		{ $$ = $3;  incomm($3, $4); }
	| common opt_comma comblock opt_comma var
		{ $$ = $3;  incomm($3, $5); }
	| common SCOMMA var
		{ incomm($1, $3); }
	;

comblock:  SCONCAT
		{ $$ = comblock(0, 0); }
	| SSLASH SNAME SSLASH
		{ $$ = comblock(toklen, token); }
	;

external: SEXTERNAL in_dcl name
		{ setext($3); }
	| external SCOMMA name
		{ setext($3); }
	;

intrinsic:  SINTRINSIC in_dcl name
		{ setintr($3); }
	| intrinsic SCOMMA name
		{ setintr($3); }
	;

equivalence:  SEQUIV in_dcl equivset
	| equivalence SCOMMA equivset
	;

equivset:  SLPAR equivlist SRPAR
		{
		struct equivblock *p;
		if(nequiv >= MAXEQUIV)
			error("too many equivalences",0,0,FATAL);
		p  =  & eqvclass[nequiv++];
		p->eqvinit = 0;
		p->eqvbottom = 0;
		p->eqvtop = 0;
		p->equivs = $2;
		}
	;

equivlist:  lhs
		{ $$ = ALLOC(eqvchain); $$->eqvitem = $1; }
	| equivlist SCOMMA lhs
		{ $$ = ALLOC(eqvchain); $$->eqvitem = $3; $$->nextp = $1; }
	;

data:	  SDATA in_data datalist
	| data opt_comma datalist
	;

in_data:
		{ if(parstate == OUTSIDE)
			{
			newproc();
			startproc(0, CLMAIN);
			}
		  if(parstate < INDATA)
			{
			enddcl();
			parstate = INDATA;
			}
		}
	;

datalist:  datavarlist SSLASH vallist SSLASH
		{ ftnint junk;
		  if(nextdata(&junk,&junk) != NULL)
			{
			error("too few initializers",0,0,ERR);
			curdtp = NULL;
			}
		  frdata($1);
		  frrpl();
		}
	;

vallist:  { toomanyinit = NO; }  val
	| vallist SCOMMA val
	;

val:	  value
		{ dataval(NULL, $1); }
	| simple SSTAR value
		{ dataval($1, $3); }
	;

value:	  simple
	| addop simple
		{ if( $1==OPMINUS && ISCONST($2) )
			consnegop($2);
		  $$ = $2;
		}
	| complex_const
	| bit_const
	;

savelist: saveitem
	| savelist SCOMMA saveitem
	;

saveitem: name
		{ int k;
		  $1->vsave = 1;
		  k = $1->vstg;
		if( ! ONEOF(k, M(STGUNKNOWN)|M(STGBSS)|M(STGINIT)) )
			error("can only save static variables", $1, 0, DCLERR);
		}
	| comblock
		{ $1->extsave = 1; }
	;

paramlist:  paramitem
	| paramlist SCOMMA paramitem
	;

paramitem:  name SEQUALS expr
		{ if($1->vclass == CLUNKNOWN)
			{ $1->vclass = CLPARAM;
			  $1->paramval = $3;
			}
		  else error("cannot make %s parameter", $1, 0, DCLERR);
		}
	;

var:	  name dims
		{ if(ndim>0) setbound($1, ndim, dims); }
	;

datavar:	  lhs
		{ ptr np;
		  vardcl(np = $1->namep);
		  if(np->vstg == STGBSS)
			np->vstg = STGINIT;
		  else if(np->vstg == STGCOMMON)
			extsymtab[np->vardesc.varno].extinit = YES;
		  else if(np->vstg==STGEQUIV)
			eqvclass[np->vardesc.varno].eqvinit = YES;
		  else if(np->vstg != STGINIT)
			error("inconsistent storage classes", np, 0, DCLERR);
		  $$ = mkchain($1, 0);
		}
	| SLPAR datavarlist SCOMMA dospec SRPAR
		{ chainp p; struct impldoblock *q;
		q = ALLOC(impldoblock);
		q->tag = TIMPLDO;
		q->varnp = $4->datap;
		p = $4->nextp;
		if(p)  { q->implb = p->datap; p = p->nextp; }
		if(p)  { q->impub = p->datap; p = p->nextp; }
		if(p)  { q->impstep = p->datap; p = p->nextp; }
		frchain( & ($4) );
		$$ = mkchain(q, 0);
		q->datalist = hookup($2, $$);
		}
	;

datavarlist: datavar
		{ curdtp = $1; curdtelt = 0; }
	| datavarlist SCOMMA datavar
		{ $$ = hookup($1, $3); }
	;

dims:
		{ ndim = 0; }
	| SLPAR dimlist SRPAR
	;

dimlist:   { ndim = 0; }   dim
	| dimlist SCOMMA dim
	;

dim:	  ubound
		{ dims[ndim].lb = 0;
		  dims[ndim].ub = $1;
		  ++ndim;
		}
	| expr SCOLON ubound
		{ dims[ndim].lb = $1;
		  dims[ndim].ub = $3;
		  ++ndim;
		}
	;

ubound:	  SSTAR
		{ $$ = 0; }
	| expr
	;

labellist: label
		{ nstars = 1; labarray[0] = $1; }
	| labellist SCOMMA label
		{ if(nstars < MAXLABLIST)  labarray[nstars++] = $3; }
	;

label:	  labelval
		{ if($1->labinacc)
			error("illegal branch to inner block, statement %s",
				convic( (ftnint) ($1->stateno) ),0,WARN1);
		  else if($1->labdefined == NO)
			$1->blklevel = blklevel;
		  $1->labused = YES;
		}
	;

labelval:   SICON
		{ $$ = mklabel( convci(toklen, token) ); }
	;

implicit:  SIMPLICIT in_dcl implist
	| implicit SCOMMA implist
	;

implist:  imptype SLPAR letgroups SRPAR
	;

imptype:   { needkwd = 1; } type
		{ vartype = $2; }
	;

letgroups: letgroup
	| letgroups SCOMMA letgroup
	;

letgroup:  letter
		{ setimpl(vartype, varleng, $1, $1); }
	| letter SMINUS letter
		{ setimpl(vartype, varleng, $1, $3); }
	;

letter:  SNAME
		{ if(toklen!=1 || token[0]<'a' || token[0]>'z')
			{
			error("implicit item must be single letter", 0, 0, DCLERR);
			$$ = 0;
			}
		  else $$ = token[0];
		}
	;

in_dcl:
		{ switch(parstate)	
			{
			case OUTSIDE:	newproc();
					startproc(0, CLMAIN);
			case INSIDE:	parstate = INDCL;
			case INDCL:	break;

			default:
				error("declaration among executables", 0, 0, DCLERR);
			}
		}
	;
