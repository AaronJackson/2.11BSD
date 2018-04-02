/*
 * Hacked for rrn
 */

/* $Header: addng.c,v 4.3.1.2 85/05/29 09:06:24 lwall Exp $
 *
 * $Log:	addng.c,v $
 * Revision 4.3.1.2  85/05/29  09:06:24  lwall
 * New newsgroups without spool directories incorrectly classified as "ancient".
 * 
 * Revision 4.3.1.1  85/05/10  11:30:50  lwall
 * Branch for patches.
 * 
 * Revision 4.3  85/05/01  11:34:41  lwall
 * Baseline for release with 4.3bsd.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "rn.h"
#include "ngdata.h"
#include "last.h"
#include "util.h"
#include "intrp.h"
#include "only.h"
#include "rcstuff.h"
#include "INTERN.h"
#include "addng.h"

void
addng_init()
{
    ;
}

#ifdef FINDNEWNG
/* generate a list of new newsgroups from active file */

bool
newlist(munged,checkinlist)
bool munged;			/* are we scanning the whole file? */
bool checkinlist;
{
    char *tmpname;
    register char *s, *status;
    register NG_NUM ngnum;
#ifndef ACTIVE_TIMES
    long birthof();
#endif
    extern char int_count;

    tmpname = filexp("/tmp/rnew.%$");
    tmpfp = fopen(tmpname,"w+");
    if (tmpfp == Nullfp) {
	printf(cantcreate,tmpname) FLUSH;
	return FALSE;
    }
    UNLINK(tmpname);			/* be nice to the world */

    while (fgets(buf,LBUFLEN,actfp) != Nullch) {
	/* Check if they want to break out of the new newsgroups search */
	if (int_count) {
	    int_count = 0;
	    fclose(tmpfp);
	    return FALSE;
	}
	if (s = index(buf,' ')) {
	    status=s;
	    while (isdigit(*status) || isspace(*status)) status++;
	    *s++ = '\0';
	    if (strnEQ(buf,"to.",3) || *status == 'x' || *status == '=')
	        /* since = groups are refiling to another group, just
		   ignore their existence */
		continue;
#ifdef ACTIVE_TIMES
	    if (inlist(buf) && ((ngnum = find_ng(buf)) == nextrcline
				|| toread[ngnum] == TR_UNSUB)
#else
	    if (checkinlist ?
		(inlist(buf) && ((ngnum = find_ng(buf)) == nextrcline
				 || toread[ngnum] == TR_UNSUB))
	      : (find_ng(buf) == nextrcline
		 && birthof(buf,(ART_NUM)atol(s)) > lasttime)
#endif
	    ) {
					/* if not in .newsrc and younger */
					/* than the last time we checked */
		fprintf(tmpfp,"%s\n",buf);
					/* then remember said newsgroup */
	    }
#ifdef FASTNEW
	    else {			/* not really a new group */
		if (!munged) {		/* did we assume not munged? */
		    fclose(tmpfp);	/* then go back, knowing that */
		    return TRUE;	/* active file was indeed munged */
		}
	    }
#endif
	}
#ifdef DEBUGGING
	else
	    printf("Bad active record: %s\n",buf) FLUSH;
#endif
    }

    /* we have successfully generated the list */

    fseek(tmpfp,0L,0);			/* rewind back to the beginning */
    while (fgets(buf,LBUFLEN,tmpfp) != Nullch) {
	buf[strlen(buf)-1] = '\0';
	get_ng(buf,TRUE);		/* add newsgroup, maybe */
    }
    fclose(tmpfp);			/* be nice to ourselves */
    return FALSE;			/* do not call us again */
}

/* return creation time of newsgroup */

long
birthof(ngnam,ngsize)
char *ngnam;
ART_NUM ngsize;
{
    char tst[128];
    long time();
 
#ifdef SERVER

    return (ngsize < 2 ? time(0) : 0);

#else not SERVER

    sprintf(tst, ngsize ? "%s/%s/1" : "%s/%s" ,spool,getngdir(ngnam));
    if (stat(tst,&filestat) < 0)
	return (ngsize ? 0L : time(Null(long *)));
	/* not there, assume something good */
    else
	return filestat.st_mtime;

#endif
}

bool
scanactive()
{
    NG_NUM oldnext = nextrcline;	/* remember # lines in newsrc */

    fseek(actfp,0L,0);
    newlist(TRUE,TRUE);
    if (nextrcline != oldnext) {	/* did we add any new groups? */
	return TRUE;
    }
    return FALSE;
}

#endif

