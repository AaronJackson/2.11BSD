#
/*
 * pi - Pascal interpreter code translator
 *
 * Charles Haley, Bill Joy UCB
 * Version 1.2 January 1979
 *
 *
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"
#include "yy.h"

#ifdef PXP
extern	int yyline;
extern	char errout;
#endif

char	errpfx	= 'E';
extern	int yyline;
/*
 * Panic is called when impossible
 * (supposedly, anyways) situations
 * are encountered.
#ifdef PI
 * Panic messages should be short
 * as they do not go to the message
 * file.
#endif
 */
panic(s)
	char *s;
{

#ifdef DEBUG
	printf("Snark (%s) line=%d yyline=%d\n", s, line, yyline);
#endif
#ifdef PXP
	Perror( "Snark in pxp", s);
#endif
#ifdef PI
	Perror( "Snark in pi", s);
#endif
	pexit(DIED);
}

extern	char *errfile;
/*
 * Error is called for
 * semantic errors and
 * prints the error and
 * a line number.
 */
error(a1, a2, a3, a4)
{
#ifdef PI
	char buf[256];
	register int i;
#endif
#ifdef PXP
	int ofout;
#endif

	if (errpfx == 'w' && opt('w') != 0)
		return;
#ifdef PXP
	flush();
	ofout = fout[0];
	fout[0] = errout;
#endif
#ifdef PI
	Enocascade = 0;
	geterr(a1, buf);
	a1 = buf;
#endif
	if (line < 0)
		line = -line;
	yySsync();
	yysetfile(filename);
#ifdef PI
	if (errpfx == ' ') {
		printf("  ");
		for (i = line; i >= 10; i /= 10)
			putchar(' ');
		printf("... ");
	} else if (Enoline)
		printf("  %c - ", errpfx);
	else
#endif
		printf("%c %d - ", errpfx, line);
	printf(a1, a2, a3, a4);
	if (errpfx == 'E')
#ifdef PI
		eflg++, cgenflg++;
#endif
#ifdef PXP
		eflg++;
#endif
	errpfx = 'E';
#ifdef PI
	if (Eholdnl)
		Eholdnl = 0;
	else
#endif
		putchar('\n');
#ifdef PXP
	flush();
	fout[0] = ofout;
#endif
}

#ifdef PI
cerror(a1, a2, a3, a4)
{

	if (Enocascade)
		return;
	setpfx(' ');
	error(a1, a2, a3, a4);
}
#endif
