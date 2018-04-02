/* $Header: /home/hyperion/mu/christos/src/sys/tcsh-6.00/RCS/tc.bind.c,v 3.0 1991/07/04 21:49:28 christos Exp $ */
/*
 * tc.bind.c: Key binding functions
 */
/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "config.h"
#if !defined(lint) && !defined(pdp11)
static char *rcsid() 
    { return "$Id: tc.bind.c,v 3.0 1991/07/04 21:49:28 christos Exp $"; }
#endif

#include "sh.h"
#include "ed.h"
#include "ed.defns.h"

static	int    str7cmp		__P((char *, char *));
static	int    tocontrol	__P((int));
static	char  *u_p_key	__P((int));
static	KEYCMD getkeycmd	__P((Char **));
static	int    parsekey		__P((Char **));
static	void   prntky		__P((KEYCMD *, Char *));
static	KEYCMD parsecmd		__P((Char *));
static	Char  *parsestring	__P((Char *, Char *));
static	void   print_all_keys	__P((void));
static	void   prntkys	__P((KEYCMD *, int, int));
static	void   bindkey_usage	__P((void));
static	void   list_functions	__P((void));
static	void   pkeys		__P((int, int));

extern int MapsAreInited;

/* like strcmp, but compairisons are striped to 7 bits
   (due to shell stupidness) */
static int
str7cmp(a, b)
    register char *a, *b;
{
    while ((*a & TRIM) == (*b++ & TRIM))
	if (!*a++)
	    return (0);
    b--;
    return ((*a & TRIM) - (*b & TRIM));
}
static int
tocontrol(c)
    int    c;
{
    c &= CHAR;
    if (Islower(c))
	c = Toupper(c);
    else if (c == ' ')
	c = '@';
    if (c == '?')
	c = 0177;
    else
	c &= 037;
    return (c);
}

static char *
u_p_key(c)			/* 'c' -> "c", '^C' -> "^" + "C" */
    register int c;
{
    register char *cp;
    static char tmp[10];

    cp = tmp;

    if (c & 0400) {
	*cp++ = 'A';
	*cp++ = '-';
	c &= 0377;
    }
    if ((c & META) && !(Isprint(c) || Iscntrl(c) && Isprint(c | 0100))) {
	*cp++ = 'M';
	*cp++ = '-';
	c &= ASCII;
    }
    if (Isprint(c)) {
	*cp++ = c;
	*cp = '\0';
	return (tmp);
    }
    else if (c == ' ') {
	(void) strcpy(cp, "Spc");
	return (tmp);
    }
    else if (c == '\n') {
	(void) strcpy(cp, "Lfd");
	return (tmp);
    }
    else if (c == '\r') {
	(void) strcpy(cp, "Ret");
	return (tmp);
    }
    else if (c == '\t') {
	(void) strcpy(cp, "Tab");
	return (tmp);
    }
    else if (c == '\033') {
	(void) strcpy(cp, "Esc");
	return (tmp);
    }
    else if (c == '\177') {
	(void) strcpy(cp, "Del");
	return (tmp);
    }
    else {
	*cp++ = '^';
	if (c == '\177') {
	    *cp++ = '?';
	}
	else {
	    *cp++ = c | 0100;
	}
	*cp = '\0';
	return (tmp);
    }
}

static  KEYCMD
getkeycmd(sp)
    Char  **sp;
{
    register Char *s = *sp;
    register char c;
    register KEYCMD keycmd = F_UNASSIGNED;
    KEYCMD *map;
    int     meta = 0;
    Char   *ret_sp = s;

    map = CcKeyMap;

    while (*s) {
	if (*s == '^' && s[1]) {
	    s++;
	    c = tocontrol(*s++);
	}
	else
	    c = *s++;

	if (*s == '\0')
	    break;

	switch (map[c | meta]) {
	case F_METANEXT:
	    meta = META;
	    keycmd = F_METANEXT;
	    ret_sp = s;
	    break;

	case F_XKEY:
	    keycmd = F_XKEY;
	    ret_sp = s;
	    /* FALLTHROUGH */

	default:
	    *sp = ret_sp;
	    return (keycmd);

	}
    }
    *sp = ret_sp;
    return (keycmd);
}

static int
parsekey(sp)
    Char  **sp;			/* Return position of first u_p_d character
				 * for return value -2 (xkeynext) */
{
    register int c, meta = 0, control = 0, ctrlx = 0;
    Char   *s = *sp;
    KEYCMD  keycmd;

    if (s == NULL) {
	xprintf("bad key specification -- null string\n");
	return -1;
    }
    if (*s == 0) {
	xprintf("bad key specification -- empty string\n");
	return -1;
    }

    (void) strip(s);		/* trim to 7 bits. */

    if (s[1] == 0)		/* single char */
	return (s[0] & 0377);

    if ((s[0] == 'F' || s[0] == 'f') && s[1] == '-') {
	if (s[2] == 0) {
	    xprintf("Bad function-key specification.  Null key not allowed\n");
	    return (-1);
	}
	*sp = s + 2;
	return (-2);
    }

    if (s[0] == '0' && s[1] == 'x') {	/* if 0xn, then assume number */
	c = 0;
	for (s += 2; *s; s++) {	/* convert to hex; skip the first 0 */
	    c *= 16;
	    if (!Isxdigit(*s)) {
		xprintf("bad key specification -- malformed hex number\n");
		return -1;	/* error */
	    }
	    if (Isdigit(*s))
		c += *s - '0';
	    else if (*s >= 'a' && *s <= 'f')
		c += *s - 'a' + 0xA;
	    else if (*s >= 'F' && *s <= 'F')
		c += *s - 'A' + 0xA;
	}
    }
    else if (s[0] == '0' && Isdigit(s[1])) {	/* if 0n, then assume number */
	c = 0;
	for (s++; *s; s++) {	/* convert to octal; skip the first 0 */
	    if (!Isdigit(*s) || *s == '8' || *s == '9') {
		xprintf("bad key specification -- malformed octal number\n");
		return -1;	/* error */
	    }
	    c = (c * 8) + *s - '0';
	}
    }
    else if (Isdigit(s[0]) && Isdigit(s[1])) {	/* decimal number */
	c = 0;
	for (; *s; s++) {	/* convert to octal; skip the first 0 */
	    if (!Isdigit(*s)) {
		xprintf("bad key specification -- malformed decimal number\n");
		return -1;	/* error */
	    }
	    c = (c * 10) + *s - '0';
	}
    }
    else {
	keycmd = getkeycmd(&s);

	if ((s[0] == 'X' || s[0] == 'x') && s[1] == '-') {	/* X- */
	    ctrlx++;
	    s += 2;
	    keycmd = getkeycmd(&s);
	}
	if ((*s == 'm' || *s == 'M') && s[1] == '-') {	/* meta */
	    meta++;
	    s += 2;
	    keycmd = getkeycmd(&s);
	}
	else if (keycmd == F_METANEXT && *s) {	/* meta */
	    meta++;
	    keycmd = getkeycmd(&s);
	}
	if (*s == '^' && s[1]) {
	    control++;
	    s++;
	    keycmd = getkeycmd(&s);
	}
	else if ((*s == 'c' || *s == 'C') && s[1] == '-') {	/* control */
	    control++;
	    s += 2;
	    keycmd = getkeycmd(&s);
	}

	if (keycmd == F_XKEY) {
	    if (*s == 0) {
		xprintf("Bad function-key specification.\n");
		xprintf("Null key not allowed\n");
		return (-1);
	    }
	    *sp = s;
	    return (-2);
	}

	if (s[1] != 0) {	/* if symbolic name */
	    char   *ts;

	    ts = short2str(s);
	    if (!str7cmp(ts, "space") || !str7cmp(ts, "Spc"))
		c = ' ';
	    else if (!str7cmp(ts, "return") || !str7cmp(ts, "Ret"))
		c = '\r';
	    else if (!str7cmp(ts, "newline") || !str7cmp(ts, "Lfd"))
		c = '\n';
	    else if (!str7cmp(ts, "linefeed"))
		c = '\n';
	    else if (!str7cmp(ts, "tab"))
		c = '\t';
	    else if (!str7cmp(ts, "escape") || !str7cmp(ts, "Esc"))
		c = '\033';
	    else if (!str7cmp(ts, "backspace"))
		c = '\b';
	    else if (!str7cmp(ts, "delete"))
		c = '\177';
	    else {
		xprintf("bad key specification -- unknown name \"%s\"\n", s);
		return -1;	/* error */
	    }
	}
	else
	    c = *s;		/* just a single char */

	if (control)
	    c = tocontrol(c);
	if (meta)
	    c |= META;
	if (ctrlx)
	    c |= 0400;
    }
    return (c & 0777);
}


void
dobindkey(v)
    Char  **v;
{
    KEYCMD *map;
    int     string, no, remove;
    Char   *par;
    Char    p;
    Char    inbuf[200];
    Char    outbuf[200];
    Char   *in;
    Char   *out;
    KEYCMD  cmd;

    if (!MapsAreInited)
	ed_IMaps();

    map = CcKeyMap;
    string = 0;
    remove = 0;
    for (no = 1, par = v[no]; 
	 par != NULL && (*par++ & CHAR) == '-'; no++, par = v[no]) {
	if ((p = (*par & CHAR)) == '-') {
	    break;
	}
	else if (p == 'a') {
	    map = CcAltMap;
	}
	else if (p == 's') {
	    string = 1;
	}
	else if (p == 'r') {
	    remove = 1;
	}
	else if (p == 'v') {
	    ed_IVIMaps();
	    return;
	}
	else if (p == 'e') {
	    ed_IEmacsMaps();
	    return;
	}
	else if (p == 'd') {
#ifdef VIDEFAULT
	    ed_IVIMaps();
#else
	    ed_IEmacsMaps();
#endif
	    return;
	}
	else if (p == 'l') {
	    list_functions();
	    return;
	}
	else {
	    bindkey_usage();
	    return;
	}
    }

    if (!v[no]) {
	print_all_keys();
	return;
    }

    if ((in = parsestring(v[no++], inbuf)) == NULL)
	return;
    if (remove) {
	if (in[1]) {
	    (void) DeleteXkey(in);
	}
	else if (map[(unsigned char) *in] == F_XKEY) {
	    (void) DeleteXkey(in);
	    map[(unsigned char) *in] = F_UNASSIGNED;
	}
	else {
	    map[(unsigned char) *in] = F_UNASSIGNED;
	}
	return;
    }
    if (!v[no]) {
	prntky(map, in);
	return;
    }
    if (v[no + 1]) {
	bindkey_usage();
	return;
    }
    if (string) {
	if ((out = parsestring(v[no], outbuf)) == NULL)
	    return;
	AddXkey(in, out);
	map[(unsigned char) *in] = F_XKEY;
    }
    else {
	if ((cmd = parsecmd(v[no])) == 0)
	    return;
	if (in[1]) {
	    AddXKeyCmd(in, (Char) cmd);
	    map[(unsigned char) *in] = F_XKEY;
	}
	else {
	    (void) ClearXkey(map, in);
	    map[(unsigned char) *in] = cmd;
	}
    }
}

static void
prntky(map, in)
    KEYCMD *map;
    Char   *in;
{
    unsigned char outbuf[100];
    register struct KeyFuncs *fp;

    if (in[0] == 0 || in[1] == 0) {
	(void) u_p_string(in, outbuf);
	for (fp = FuncNames; fp->name; fp++) {
	    if (fp->func == map[(unsigned char) *in]) {
		xprintf("%s\t->\t%s\n", outbuf, fp->name);
	    }
	}
    }
    else {
	(void) PrintXkey(in);
    }
}

static  KEYCMD
parsecmd(str)
    Char   *str;
{
    register struct KeyFuncs *fp;

    for (fp = FuncNames; fp->name; fp++) {
	if (str7cmp(short2str(str), fp->name) == 0) {
	    return fp->func;
	}
    }
    xprintf("Bad command name: %s\n", short2str(str));
    return 0;
}

int
parseescape(ptr)
    Char  **ptr;
{
    Char   *p, c;

    p = *ptr;

    if ((p[1] & CHAR) == 0) {
	xprintf("Something must follow: %c\\n", *p);
	return 0;
    }
    if ((*p & CHAR) == '\\') {
	p++;
	switch (*p & CHAR) {
	case 'a':
	    c = '\007';		/* Bell */
	    break;
	case 'b':
	    c = '\010';		/* Backspace */
	    break;
	case 't':
	    c = '\011';		/* Horizontal Tab */
	    break;
	case 'n':
	    c = '\012';		/* New Line */
	    break;
	case 'v':
	    c = '\013';		/* Vertical Tab */
	    break;
	case 'f':
	    c = '\014';		/* Form Feed */
	    break;
	case 'r':
	    c = '\015';		/* Carriage Return */
	    break;
	case 'e':
	    c = '\033';		/* Escape */
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	    {
		register int cnt, val, ch;

		for (cnt = 0, val = 0; cnt < 3; cnt++) {
		    ch = *p++ & CHAR;
		    if (ch < '0' || ch > '7') {
			p--;
			break;
		    }
		    val = (val << 3) | (ch - '0');
		}
		if ((val & 0xffffff00) != 0) {
		    xprintf("Octal constant does not fit in a char.\n");
		    return 0;
		}
		c = val;
		--p;
	    }
	    break;
	default:
	    c = *p;
	    break;
	}
    }
    else if ((*p & CHAR) == '^') {
	p++;
	c = (*p == '?') ? '\177' : ((*p & CHAR) & 0237);
    }
    else
	c = *p;
    *ptr = p;
    return (c);
}

static Char *
parsestring(str, buf)
    Char   *str;
    Char   *buf;
{
    Char   *b;
    Char   *p;

    b = buf;
    if (*str == 0) {
	xprintf("Null string specification\n");
	return 0;
    }

    for (p = str; *p != 0; p++) {
	if ((*p & CHAR) == '\\' || (*p & CHAR) == '^') {
	    if ((*b++ = parseescape(&p)) == 0)
		return 0;
	}
	else {
	    *b++ = *p & CHAR;
	}
    }
    *b = 0;
    return buf;
}

unsigned char *
u_p_string(str, buf)
    Char   *str;
    unsigned char *buf;
{
    unsigned char *b;
    Char   *p;

    b = buf;
    *b++ = '"';
    if (*str == 0) {
	*b++ = '^';
	*b++ = '@';
	*b++ = '"';
	*b++ = 0;
	return buf;
    }

    for (p = str; *p != 0; p++) {
	if (Iscntrl(*p)) {
	    *b++ = '^';
	    if (*p == '\177')
		*b++ = '?';
	    else
		*b++ = *p | 0100;
	}
	else if (*p == '^' || *p == '\\') {
	    *b++ = '\\';
	    *b++ = *p;
	}
	else if (*p == ' ' || (Isprint(*p) && !Isspace(*p))) {
	    *b++ = *p;
	}
	else {
	    *b++ = '\\';
	    *b++ = ((*p >> 6) & 7) + '0';
	    *b++ = ((*p >> 3) & 7) + '0';
	    *b++ = (*p & 7) + '0';
	}
    }
    *b++ = '"';
    *b++ = 0;
    return buf;			/* should check for overflow */
}

static void
print_all_keys()
{
    int     prev, i;

    xprintf("Standard key bindings\n");
    prev = 0;
    for (i = 0; i < 256; i++) {
	if (CcKeyMap[prev] == CcKeyMap[i])
	    continue;
	prntkys(CcKeyMap, prev, i - 1);
	prev = i;
    }
    prntkys(CcKeyMap, prev, i - 1);

    xprintf("Alternative key bindings\n");
    prev = 0;
    for (i = 0; i < 256; i++) {
	if (CcAltMap[prev] == CcAltMap[i])
	    continue;
	prntkys(CcAltMap, prev, i - 1);
	prev = i;
    }
    prntkys(CcAltMap, prev, i - 1);
    xprintf("Multi-character bindings\n");
    (void) PrintXkey(STRNULL);	/* print all Xkey bindings */
}

static void
prntkys(map, first, last)
    KEYCMD *map;
    int     first, last;
{
    register struct KeyFuncs *fp;
    Char    firstbuf[2], lastbuf[2];
    unsigned char unparsbuf[10], extrabuf[10];

    firstbuf[0] = first;
    firstbuf[1] = 0;
    lastbuf[0] = last;
    lastbuf[1] = 0;
    if (map[first] == F_UNASSIGNED) {
	if (first == last)
	    xprintf("%-15s->  is undefined\n",
		    u_p_string(firstbuf, unparsbuf));
	return;
    }

    for (fp = FuncNames; fp->name; fp++) {
	if (fp->func == map[first]) {
	    if (first == last) {
		xprintf("%-15s->  %s\n",
			u_p_string(firstbuf, unparsbuf), fp->name);
	    }
	    else {
		xprintf("%-4s to %-7s->  %s\n",
			u_p_string(firstbuf, unparsbuf),
			u_p_string(lastbuf, extrabuf), fp->name);
	    }
	    return;
	}
    }
    if (map == CcKeyMap) {
	xprintf("BUG!!! %s isn't bound to anything.\n",
		u_p_string(firstbuf, unparsbuf));
	xprintf("CcKeyMap[%d] == %d\n", first, CcKeyMap[first]);
    }
    else {
	xprintf("BUG!!! %s isn't bound to anything.\n",
		u_p_string(firstbuf, unparsbuf));
	xprintf("CcAltMap[%d] == %d\n", first, CcAltMap[first]);
    }
}

static void
bindkey_usage()
{
    xprintf(
	"Usage: bindkey [options] [--] [in-string [out-string | command]]\n");
    xprintf("    -a   bind key in alternative key binding\n");
    xprintf("    -s   bind an out-string instad of a command\n");
    xprintf("    -v   initialized maps to default vi bindings\n");
    xprintf("    -e   initialized maps to default emacs bindings\n");
    xprintf("    -d   initialized maps to default bindings\n");
#ifdef	LONGFUNCS
    xprintf("    -l   list available functions with descriptions\n");
#else
    xprintf("    -l   list available functions\n");
#endif	/* LONGFUNCS */
    xprintf("    -r   remove the binding of in-string\n");
    xprintf(
       "\nIn no out-string or command is given, the binding for in-string\n");
    xprintf("is printed or all bindings if in-strings is not given.\n");
}

static void
list_functions()
{
    register struct KeyFuncs *fp;

    for (fp = FuncNames; fp->name; fp++) {
#ifdef	LONGFUNCS
	xprintf("%s\n          %s\n", fp->name, fp->description);
#else
	xprintf("%s\n", fp->name);
#endif
    }
}

void
dobind(v)
    register Char **v;
{
    register int c;
    register struct KeyFuncs *fp;
    register int i, prev;
    Char   *p, *l;
    Char    buf[1000];

    /*
     * Assume at this point that i'm given 2 or 3 args - 'bind', the f-name,
     * and the key; or 'bind' key to print the func for that key.
     */

    if (!MapsAreInited)
	ed_IMaps();

    if (v[1] && v[2] && v[3]) {
	xprintf(
	   "usage: bind [KEY | COMMAND KEY | \"emacs\" | \"vi\" | \"-a\"]\n");
	return;
    }

    if (v[1] && v[2]) {		/* if bind FUNCTION KEY */
	for (fp = FuncNames; fp->name; fp++) {
	    if (str7cmp(short2str(v[1]), fp->name) == 0) {
		Char   *s = v[2];

		if ((c = parsekey(&s)) == -1)
		    return;
		if (c == -2) {	/* extented key */
		    for (i = 0; i < 256; i++) {
			if (i != 033 && (CcKeyMap[i] == F_XKEY ||
					 CcAltMap[i] == F_XKEY)) {
			    p = buf;
			    if (i > 0177) {
				*p++ = 033;
				*p++ = i & ASCII;
			    }
			    else {
				*p++ = i;
			    }
			    for (l = s; *l != 0; l++) {
				*p++ = *l;
			    }
			    *p = 0;
			    AddXKeyCmd(buf, fp->func);
			}
		    }
		    return;
		}
		if (c & 0400) {
		    if (VImode) {
			CcAltMap[c & 0377] = fp->func;	
			/* bind the vi cmd mode key */
			if (c & META) {
			    buf[0] = 033;
			    buf[1] = c & ASCII;
			    buf[2] = 0;
			    AddXKeyCmd(buf, fp->func);
			}
		    }
		    else {
			buf[0] = 030;	/* ^X */
			buf[1] = c & 0377;
			buf[2] = 0;
			AddXKeyCmd(buf, fp->func);
			CcKeyMap[030] = F_XKEY;
		    }
		}
		else {
		    CcKeyMap[c] = fp->func;	/* bind the key */
		    if (c & META) {
			buf[0] = 033;
			buf[1] = c & ASCII;
			buf[2] = 0;
			AddXKeyCmd(buf, fp->func);
		    }
		}
		return;
	    }
	}
	stderror(ERR_NAME | ERR_STRING, "Invalid function");
    }
    else if (v[1]) {
	char   *cv = short2str(v[1]);

	if (str7cmp(cv, "list") == 0) {
	    for (fp = FuncNames; fp->name; fp++) {
		xprintf("%s\n", fp->name);
	    }
	    return;
	}
	if ((str7cmp(cv, "emacs") == 0) ||
#ifndef VIDEFAULT
	    (str7cmp(cv, "defaults") == 0) ||
	    (str7cmp(cv, "default") == 0) ||
#endif
	    (str7cmp(cv, "mg") == 0) ||
	    (str7cmp(cv, "gnumacs") == 0)) {
	    /* reset keys to default */
	    ed_IEmacsMaps();
#ifdef VIDEFAULT
	}
	else if ((str7cmp(cv, "vi") == 0)
		 || (str7cmp(cv, "default") == 0)
		 || (str7cmp(cv, "defaults") == 0)) {
#else
	}
	else if (str7cmp(cv, "vi") == 0) {
#endif
	    ed_IVIMaps();
	}
	else {			/* want to know what this key does */
	    Char   *s = v[1];

	    if ((c = parsekey(&s)) == -1)
		return;
	    if (c == -2) {	/* extended key */
		(void) PrintXkey(s);
		return;
	    }
	    pkeys(c, c);	/* must be regular key */
	}
    }
    else {			/* list all the bindings */
	prev = 0;
	for (i = 0; i < 256; i++) {
	    if (CcKeyMap[prev] == CcKeyMap[i])
		continue;
	    pkeys(prev, i - 1);
	    prev = i;
	}
	pkeys(prev, i - 1);
	prev = 0;
	for (i = 256; i < 512; i++) {
	    if (CcAltMap[prev & 0377] == CcAltMap[i & 0377])
		continue;
	    pkeys(prev, i - 1);
	    prev = i;
	}
	pkeys(prev, i - 1);
	(void) PrintXkey(STRNULL);	/* print all Xkey bindings */
    }
    return;
}

static void
pkeys(first, last)
    register int first, last;
{
    register struct KeyFuncs *fp;
    register KEYCMD *map;
    char    buf[8];

    if (last & 0400) {
	map = CcAltMap;
	first &= 0377;
	last &= 0377;
    }
    else {
	map = CcKeyMap;
    }
    if (map[first] == F_UNASSIGNED) {
	if (first == last)
#ifdef _SEQUENT_
	    xprintf(" %s\t\tis undefined\n",
		    u_p_key(map == CcAltMap ? first | 0400 : first));
#else				/* _SEQUENT_ */
	    xprintf(" %s\t\tis undefined\n", u_p_key(first));
#endif				/* _SEQUENT_ */
	return;
    }

    for (fp = FuncNames; fp->name; fp++) {
	if (fp->func == map[first]) {
	    if (first == last) {
		xprintf(" %s\t\t%s\n",
		    u_p_key((first & 0377) | (map == CcAltMap ? 0400 : 0)),
			fp->name);
	    }
	    else {
		(void) strcpy(buf, u_p_key((first & 0377) |
					      (map == CcAltMap ? 0400 : 0)));
		xprintf(" %s..%s\t\t%s\n", buf,
		     u_p_key((last & 0377) | (map == CcAltMap ? 0400 : 0)),
			fp->name);
	    }
	    return;
	}
    }
    if (map == CcKeyMap) {
	xprintf("BUG!!! %s isn't bound to anything.\n", u_p_key(first));
	xprintf("CcKeyMap[%d] == %d\n", first, CcKeyMap[first]);
    }
    else {
	xprintf("BUG!!! %s isn't bound to anything.\n",
		u_p_key(first & 0400));
	xprintf("CcAltMap[%d] == %d\n", first, CcAltMap[first]);
    }
}
