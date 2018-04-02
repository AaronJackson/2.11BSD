#
/*
 * pxp - Pascal execution profiler
 *
 * Steven Schultz GTE
 * Version 1.2.1 October 1996
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"

/*
 * This program is described in detail in the "PXP 1.0 Implementation Notes"
 *
 * The structure of pxp is very similar to that of the compiler pc.
 * The major new pieces here are a set of profile data maintenance
 * routines in the file pmon.c and a set of pretty printing utility
 * routines in the file pp.c.
 * The semantic routines of pc have been rewritten to do a simple
 * reformatting tree walk, the parsing and scanning remains
 * the same.
 *
 * This version does not reformat comments, rather discards them.
 * It also does not place more than one statement per line and
 * is not very intelligent about folding long lines, with only
 * an ad hoc way of folding case label list and enumerated type
 * declarations being implemented.
 */

char	usagestr[] =
	"pxp [ -acdefjntuw_ ] [ -23456789 ] [ -z [ name ... ] ] name.p";

char	*how_file	= "/usr/share/pascal/how_pxp";
int	how_pathlen	= 18;	/* "/usr/share/pascal/" */

char	*stdoutn	= "Standard output";

int	unit	= 4;

extern	int ibuf[259];
extern	char errout;

/*
 * Main program for pxp.
 * Process options, then call yymain
 * to do all the real work.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	register char *cp;
	register c;

	if (argv[0][0] == 'a')
		how_file += how_pathlen;
	argc--, argv++;
	if (argc == 0) {
		execl("/bin/cat", "cat", how_file, 0);
		goto usage;
	}
	while (argc > 0) {
		cp = argv[0];
		if (*cp++ != '-')
			break;
		while (c = *cp++) switch (c) {
#ifdef DEBUG
			case 'T':
				typetest++;
				continue;
			case 'A':
				testtrace++;
			case 'F':
				fulltrace++;
			case 'E':
				errtrace++;
				continue;
			case 'C':
				yycosts();
				pexit(NOSTART);
			case 'U':
				yyunique++;
				continue;
#endif
			case 'a':
				all++;
				continue;
			case 'c':
				core++;
				continue;
			case 'd':
				nodecl++;
				continue;
			case 'e':
				noinclude = -1;
				continue;
			case 'f':
				full++;
				continue;
			case 'j':
				justify++;
				continue;
			case 'l':
			case 'n':
				togopt(c);
				continue;
			case 'o':
				onefile++;
				continue;
			case 's':
				stripcomm++;
				continue;
			case 't':
				table++;
				continue;
			case 'u':
			case 'w':
				togopt(c);
				continue;
			case 'z':
				profile++;
				pflist = argv + 1;
				pflstc = 0;
				while (argc > 1) {
					if (dotted(argv[1], 'p'))
						break;
					pflstc++, argc--, argv++;
				}
				if (pflstc == 0)
					togopt(c);
				else
					nojunk++;
				continue;
			case '_':
				underline++;
				continue;
			default:
				if (c >= '2' && c <= '9') {
					unit = c - '0';
					continue;
				}
usage:
				Perror("Usage", usagestr);
				exit(1);
		}
		argc--, argv++;
	}
	if (core && !profile && !table)
		profile++;
	if (argc == 0 || argc > 2)
		goto usage;
	if (profile || table) {
		noinclude = 0;
		if (argc == 2) {
			argc--;
			getit(argv[1]);
		} else
			getit(core ? "core" : "pmon.out");
	} else
		noinclude++;
	if (argc != 1)
		goto usage;
	firstname = filename = argv[0];
	if (dotted(filename, 'i')) {
		if (profile || table)
			goto usage;
		noinclude = 1;
		bracket++;
	} else if (!dotted(filename, 'p')) {
		Perror(filename, "Name must end in '.p'");
		exit(1);
	}
	if (fopen(filename, ibuf) < 0)
		perror(filename), pexit(NOSTART);
	if (onefile) {
		int onintr();

		cp = (stdoutn = "/tmp/pxp00000") + 13;
		signal(2, onintr);
		for (c = getpid(); c; c /= 10)
			*--cp |= (c % 10);
		if (fcreat(stdoutn, fout) < 0)
bad:
			perror(stdoutn), exit(1);
		close(fout[0]);
		if (open(stdoutn, 2) != fout[0])
			goto bad;
	} else
		fout[0] = dup(1);
	if (profile || opt('l')) {
		opt('n')++;
		yysetfile(filename);
		opt('n')--;
	} else
		lastname = filename;
	errout = dup(2);
	yymain();
	/* No return */
}

/*
 * Put a header on a top of a page
 */
header()
{
	extern char version[];
	static char reenter;
	extern int outcol;

	gettime();
	if (reenter) {
		if (outcol)
			putchar('\n');
		putchar('\f');
	}
	reenter++;
	if (profile || table) {
		printf("Berkeley Pascal PXP -- Version 1.1 (%s)\n\n%s  %s\n\n", version, myctime(&tvec), filename);
		printf("Profiled %s\n\n", myctime(&ptvec));
	}
}

char	ugh[]	= "Fatal error in pxp\n";
/*
 * Exit from the Pascal system.
 * We throw in an ungraceful termination
 * message if c > 1 indicating a severe
 * error such as running out of memory
 * or an internal inconsistency.
 */
pexit(c)
	int c;
{
	register char *cp;
	extern int outcol;

	if (stdoutn[0] == '/')
		unlink(stdoutn);
	if (outcol)
		putchar('\n');
	flush();
	if (c == DIED)
		write(2, ugh, sizeof ugh);
	exit(c);
}

onintr()
{

	pexit(DIED);
}

puthedr()
{

	yysetfile(filename);
}
