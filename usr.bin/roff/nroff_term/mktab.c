


#include <stdio.h>

#include "tw.h"

char *ints[] = { "bset", "breset", "Hor", "Vert", "Newline",
	"Char", "Em", "Halfline", "Adj" };

char *chars[] = { "twinit", "twrest", "twnl", "hlr", "hlf", "flr", "bdon",
	"bdoff", "ploton", "plotoff", "up", "down", "right", "left" };

char *codes[128] = {
	"hy", "bu", "sq", "em", "ru", "14", "12", "34",	/* 200-207 */
	"mi", "fi", "fl", "ff", "Fi", "Fl", "de", "dg",	/* 210-217 */
	"sc", "fm", "aa", "ga", "ul", "sl",    0,    0,	/* 220-227 */
	"*a", "*b", "*g", "*d", "*e", "*z", "*y", "*h",	/* 230-237 */

	"*i", "*k", "*l", "*m", "*n", "*c", "*o", "*p",	/* 240-247 */
	"*r", "*s", "*t", "*u", "*f", "*x", "*q", "*w",	/* 250-257 */
	"*G", "*D", "*H", "*L", "*C", "*P", "*S",    0,	/* 260-267 */
	"*U", "*F", "*Q", "*W", "sr", "ts", "rn", ">=",	/* 270-277 */

	"<=", "==", "mi", "~=", "ap", "!=", "->", "<-",	/* 300-307 */
	"ua", "da", "eq", "mu", "di", "+-", "cu", "ca",	/* 310-317 */
	"sb", "sp", "ib", "ip", "if", "pd", "gr", "no",	/* 320-327 */
	"is", "pt", "es", "mo", "pl", "rg", "co", "br",	/* 330-337 */

	"ct", "dd", "rh", "lh", "**", "bs", "or", "ci",	/* 340-347 */
	"lt", "lb", "rt", "rb", "lk", "rk", "bv", "lf",	/* 350-357 */
	"rf", "lc", "rc",				/* 360-367 */
};


static void
escpr(unsigned char *s)
{
	while (*s != 0) {
		if (*s == '\"' || *s == '\\' || *s < 33 || *s >126) {
			printf("\\%03o", *s);
		} else
			printf("%c", *s);
		s++;
	}
}

int
main(int argc, char *argv[])
{
	int i;
	int *ip;
	unsigned char **cp;

	printf("# %s\n", argc > 1 ? argv[1] : "dummy");


	ip = (int *)&t;
	for (i = 0; i <= (&t.Adj - &t.bset); i++)
		printf("%s	%d\n", ints[i], ip[i]);

	cp = (unsigned char **)&t.twinit;
	for (i = 0; i <= (&t.left - &t.twinit); i++) {
		printf("%s	\"", chars[i]);
		escpr(cp[i]);
		printf("\"\n");
	}

	printf("# codetab\n");

	cp = (unsigned char **)&t.codetab[0];
	for (i = 128-32; i <= 256-32; i++) {
		if (cp[i] != NULL && *cp[i] != 0 && codes[i-96] != NULL)
			printf("%s 	", codes[i-96]);
		else continue;
		printf("%d ", cp[i][0] & 0177);
		escpr(cp[i]+1);
		printf("\n");
	}
	return 0;
}
