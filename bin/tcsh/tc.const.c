/* $Header: /home/hyperion/mu/christos/src/sys/tcsh-6.00/RCS/tc.const.c,v 3.0 1991/07/04 21:49:28 christos Exp $ */
/*
 * sh.const.c: String constants for tcsh.
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
    { return "$Id: tc.const.c,v 3.0 1991/07/04 21:49:28 christos Exp $"; }
#endif
#include "sh.h"

Char STRlogout[]        = { 'l', 'o', 'g', 'o', 'u', 't', '\0' };
Char STRa_logout[]    = { 'a', 'u', 't', 'o', 'l', 'o', 'g', 'o', 'u', 't', 
			    '\0' };
Char STRdflt_auto_logout[] = { '6', '0', '\0' };
Char STRa_matic[]     = { 'a', 'u', 't', 'o', 'm', 'a', 't', 'i', 'c', '\0' };
Char STRaout[]          = { 'a', '.', 'o', 'u', 't', '\0' };
Char STRtty[]           = { 't', 't', 'y', '\0' };
Char STRany[]           = { 'a', 'n', 'y', '\0' };
Char STRstatus[]        = { 's', 't', 'a', 't', 'u', 's', '\0' };
Char STR0[]             = { '0', '\0' };
Char STR1[]             = { '1', '\0' };
Char STRNULL[]          = { '\0' };
Char STRtcsh[]          = { 't', 'c', 's', 'h', '\0' };
Char STRhome[]          = { 'h', 'o', 'm', 'e', '\0' };
Char STRuser[]          = { 'u', 's', 'e', 'r', '\0' };
Char STRterm[]          = { 't', 'e', 'r', 'm', '\0' };
Char STRversion[]       = { 'v', 'e', 'r', 's', 'i', 'o', 'n', '\0' };
Char STRuid[]           = { 'u', 'i', 'd', '\0' };
Char STRgid[]           = { 'g', 'i', 'd', '\0' };
Char STRHST[]          = { 'H', 'O', 'S', 'T', '\0' };
Char STRHSTTYPE[]      = { 'H', 'O', 'S', 'T', 'T', 'Y', 'P', 'E', '\0' };
Char STRedit[]          = { 'e', 'd', 'i', 't', '\0' };
Char STRaddsuffix[]     = { 'a', 'd', 'd', 's', 'u', 'f', 'f', 'i', 'x', '\0' };
Char STRshell[]         = { 's', 'h', 'e', 'l', 'l', '\0' };
Char STRtmpsh[]         = { '/', 't', 'm', 'p', '/', 's', 'h', '\0' };
Char STRverbose[]       = { 'v', 'e', 'r', 'b', 'o', 's', 'e', '\0' };
Char STRecho[]          = { 'e', 'c', 'h', 'o', '\0' };
Char STRpath[]          = { 'p', 'a', 't', 'h', '\0' };
Char STRprompt[]        = { 'p', 'r', 'o', 'm', 'p', 't', '\0' };
Char STRp_2[]       = { 'p', 'r', 'o', 'm', 'p', 't', '2', '\0' };
Char STRp_3[]       = { 'p', 'r', 'o', 'm', 'p', 't', '3', '\0' };
Char STRcwd[]           = { 'c', 'w', 'd', '\0' };
Char STRstar[]          = { '*', '\0' };
Char STRdot[]           = { '.', '\0' };
Char STRhsto[] 	= { 'h', 'i', 's', 't', 'o', 'r', 'y', '\0' };
Char STRsource[]        = { 's', 'o', 'u', 'r', 'c', 'e', '\0' };
Char STRmh[]            = { '-', 'h', '\0' };
Char STRhstf[]      = { '~', '/', '.', 'h', 'i', 's', 't', 'o', 'r', 
		            'y', '\0' };

#ifdef CSHDIRS
Char STRdirfile[]     = { '~', '/', '.', 'c', 's', 'h', 'd', 'i', 'r',
                          's', '\0' };
Char STRs_dirs[]    = { '/', '.', 'c', 's', 'h', 'd', 'i', 'r', 's', '\0' };
Char STRsvdirs[]    = { 's', 'a', 'v', 'e', 'd', 'i', 'r', 's', '\0' };
#endif

Char STRargv[]          = { 'a', 'r', 'g', 'v', '\0' };
Char STRsvhist[]      = { 's', 'a', 'v', 'e', 'h', 'i', 's', 't', '\0' };
Char STRs_hist[]      = { '/', '.', 'h', 'i', 's', 't', 'o', 'r', 'y', '\0' };
Char STRnormal[]        = { 'n', 'o', 'r', 'm', 'a', 'l', '\0' };
Char STRs_logout[]    = { '/', '.', 'l', 'o', 'g', 'o', 'u', 't', '\0' };
Char STRjobs[]          = { 'j', 'o', 'b', 's', '\0' };
Char STRsymhash[]       = { '#', ' ', '\0' };
Char STRsymarrow[]      = { '>', ' ', '\0' };
Char STRmquestion[]     = { '%', 'R', '?' | QUOTE, ' ', '\0' };
Char STRCORRECT[]       = { 'C', 'O', 'R', 'R', 'E', 'C', 'T', '>', '%', 'R', 
			    ' ', '(', 'y', '|', 'n', ')', '?' | QUOTE, ' ', 
			    '\0' };
Char STRunalias[]       = { 'u', 'n', 'a', 'l', 'i', 'a', 's', '\0' };
Char STRalias[]         = { 'a', 'l', 'i', 'a', 's', '\0' };
Char STRprecmd[]        = { 'p', 'r', 'e', 'c', 'm', 'd', '\0' };
Char STRcwdcmd[]        = { 'c', 'w', 'd', 'c', 'm', 'd', '\0' };
Char STRperiodic[]      = { 'p', 'e', 'r', 'i', 'o', 'd', 'i', 'c', '\0' };
Char STRtperiod[]       = { 't', 'p', 'e', 'r', 'i', 'o', 'd', '\0' };
Char STRml[]		= { '-', 'l', '\0' };
Char STRslash[]		= { '/', '\0' };
Char STRdt_l[]		= { '.', '/', '\0' };
Char STRdotdotsl[]	= { '.', '.', '/', '\0' };
Char STRcdpath[]	= { 'c', 'd', 'p', 'a', 't', 'h', '\0' };
Char STRputohome[]	= { 'p', 'u', 's', 'h', 'd', 't', 'o', 'h', 'o', 'm',
			    'e', '\0' };
Char STRpusilent[]	= { 'p', 'u', 's', 'h', 'd', 's', 'i', 'l', 'e', 'n',
			    't', '\0' };
Char STRdextract[]	= { 'd', 'e', 'x', 't', 'r', 'a', 'c', 't', '\0' };
Char STRig__symlinks[] = { 'i', 'g', 'n', 'o', 'r', 'e', '_', 's', 'y', 'm',
			    'l', 'i', 'n', 'k', 's', '\0' };
Char STRchase_symlinks[] = { 'c', 'h', 'a', 's', 'e', '_', 's', 'y', 'm', 'l', 
			    'i', 'n', 'k', 's', '\0' };
Char STRPWD[]		= { 'P', 'W', 'D', '\0' };
Char STRor2[]		= { '|', '|', '\0' };
Char STRand2[]		= { '&', '&', '\0' };
Char STRor[]		= { '|', '\0' };
Char STRcaret[]		= { '^', '\0' };
Char STRand[]		= { '&', '\0' };
Char STRequal[]		= { '=', '\0' };
Char STRbang[]		= { '!', '\0' };
Char STRtilde[]		= { '~', '\0' };
Char STRLparen[]	= { '(', '\0' };
Char STRLbrace[]	= { '{', '\0' };
Char STRfkcom[]	= { '{', ' ', '.', '.', '.', ' ', '}', '\0' };
Char STRRbrace[]	= { '}', '\0' };
Char STRPATH[] 		= { 'P', 'A', 'T', 'H', '\0' };
Char STRdefault[]	= { 'd', 'e', 'f', 'a', 'u', 'l', 't', '\0' };
Char STRmn[]		= { '-', 'n', '\0' };
Char STRminus[]		= { '-', '\0' };
Char STRnoglob[]	= { 'n', 'o', 'g', 'l', 'o', 'b', '\0' };
Char STRnonomatch[]	= { 'n', 'o', 'n', 'o', 'm', 'a', 't', 'c', 'h', '\0' };
Char STRfk_com1[]	= { '`', ' ', '.', '.', '.', ' ', '`', '\0' };
Char STRampm[]		= { 'a', 'm', 'p', 'm', '\0' };
Char STRchild[]		= { 'c', 'h', 'i', 'l', 'd', '\0' };
Char STRtime[]		= { 't', 'i', 'm', 'e', '\0' };
Char STRnotify[]	= { 'n', 'o', 't', 'i', 'f', 'y', '\0' };
Char STRprintexitvalue[] = { 'p', 'r', 'i', 'n', 't', 'e', 'x', 'i', 't', 'v', 
			    'a', 'l', 'u', 'e', '\0' };
Char STRL_parensp[]	= { '(', ' ', '\0' };
Char STRspRparen[]	= { ' ', ')', '\0' };
Char STRspace[]		= { ' ', '\0' };
Char STRsP2sp[]	= { ' ', '|', '|', ' ', '\0' };
Char STRspand2sp[]	= { ' ', '&', '&', ' ', '\0' };
Char STRsPsp[]	= { ' ', '|', ' ', '\0' };
Char STRsmsp[]	= { ';', ' ', '\0' };
Char STRsm[]		= { ';', '\0' };
Char STRspL_arrow2sp[]	= { ' ', '<', '<', ' ', '\0' };
Char STRspLarrowsp[]	= { ' ', '<', ' ', '\0' };
Char STRspR_arrow2[]	= { ' ', '>', '>', '\0' };
Char STRspRarrow[]	= { ' ', '>', '\0' };
Char STRgt[]		= { '>', '\0' };
Char STRsp3dots[]	= { ' ', '.', '.', '.', '\0' };
Char STRc_2[] 	= { '%', '%', '\0' };
Char STRc_plus[] 	= { '%', '+', '\0' };
Char STRc_minus[] 	= { '%', '-', '\0' };
Char STRc_hash[] 	= { '%', '#', '\0' };
#ifdef BSDJOBS
Char STRcontinue[]	= { 'c', 'o', 'n', 't', 'i', 'n', 'u', 'e', '\0' };
Char STRcon_args[]	= { 'c', 'o', 'n', 't', 'i', 'n', 'u', 'e', '_', 'a',
			    'r', 'g', 's', '\0' };
Char STRunderpause[]	= { '_', 'p', 'a', 'u', 's', 'e', '\0' };
#endif
Char STRbKqpwd[]	= { '`', 'p', 'w', 'd', '`', '\0' };
Char STRhstc[]	= { 'h', 'i', 's', 't', 'c', 'h', 'a', 'r', 's', '\0' };
Char STRhstl[]	= { 'h', 'i', 's', 't', 'l', 'i', 't', '\0' };
Char STRUSER[]		= { 'U', 'S', 'E', 'R', '\0' };
Char STRLOGNAME[]	= { 'L', 'O', 'G', 'N', 'A', 'M', 'E', '\0' };
Char STRwordchars[]	= { 'w', 'o', 'r', 'd', 'c', 'h', 'a', 'r', 's', '\0' };
Char STRTRM[]		= { 'T', 'E', 'R', 'M', '\0' };
Char STRHOME[]		= { 'H', 'O', 'M', 'E', '\0' };
Char STRbKslash_quote[] = { 'b', 'a', 'c', 'k', 's', 'l', 'a', 's', 'h', '_',
			     'q', 'u', 'o', 't', 'e', '\0' };
Char STRRparen[]	= { ')', '\0' };
Char STRcolon[]		= { ':', '\0' };
Char STRmail[]		= { 'm', 'a', 'i', 'l', '\0' };
Char STRwatch[]		= { 'w', 'a', 't', 'c', 'h', '\0' };

Char STRd_tcshrc[]	= { '/', '.', 't', 'c', 's', 'h', 'r', 'c', '\0' };
Char STRd_cshrc[]	= { '/', '.', 'c', 's', 'h', 'r', 'c', '\0' };
Char STRd_login[]	= { '/', '.', 'l', 'o', 'g', 'i', 'n', '\0' };
Char STRig_eof[]	= { 'i', 'g', 'n', 'o', 'r', 'e', 'e', 'o', 'f', '\0' };
Char STRnoclobber[]	= { 'n', 'o', 'c', 'l', 'o', 'b', 'b', 'e', 'r', '\0' };
Char STRhelpcommand[]	= { 'h', 'e', 'l', 'p', 'c', 'o', 'm', 'm', 'a', 'n', 
			    'd', '\0' };
Char STRfignore[]	= { 'f', 'i', 'g', 'n', 'o', 'r', 'e', '\0' };
Char STRrecexact[]	= { 'r', 'e', 'c', 'e', 'x', 'a', 'c', 't', '\0' };
Char STRl_max[]	= { 'l', 'i', 's', 't', 'm', 'a', 'x', '\0' };
Char STRl_links[]	= { 'l', 'i', 's', 't', 'l', 'i', 'n', 'k', 's', '\0' };
Char STRDING[]		= { 'D', 'I', 'N', 'G', '!', '\0' };
Char STRQNULL[]		= { '\0' | QUOTE, '\0' };
Char STRcorrect[]	= { 'c', 'o', 'r', 'r', 'e', 'c', 't', '\0' };
Char STRcmd[]		= { 'c', 'm', 'd', '\0' };
Char STRall[]		= { 'a', 'l', 'l', '\0' };
Char STRa_expand[]	= { 'a', 'u', 't', 'o', 'e', 'x', 'p', 'a', 'n', 'd',
			    '\0' };
Char STRa_correct[]	= { 'a', 'u', 't', 'o', 'c', 'o', 'r', 'r', 'e', 'c',
			    't', '\0' };
Char STRa_list[]	= { 'a', 'u', 't', 'o', 'l', 'i', 's', 't', '\0' };
Char STRmatchbeep[]	= { 'm', 'a', 't', 'c', 'h', 'b', 'e', 'e', 'p', '\0' };
Char STRnever[]		= { 'n', 'e', 'v', 'e', 'r', '\0' };
Char STRnomatch[]	= { 'n', 'o', 'm', 'a', 't', 'c', 'h', '\0' };
Char STRambiguous[]	= { 'a', 'm', 'b', 'i', 'g', 'u', 'o', 'u', 's', '\0' };
Char STRnotunique[]	= { 'n', 'o', 't', 'u', 'n', 'i', 'q', 'u', 'e', '\0' };
Char STRret[]		= { '\n', '\0' };
Char STRnobeep[]	= { 'n', 'o', 'b', 'e', 'e', 'p', '\0' };
Char STRvisiblebell[]	= { 'v', 'i', 's', 'i', 'b', 'l', 'e', 'b', 'e', 'l', 
			    'l', '\0' };
Char STRrecognize_only_executables[] = { 'r', 'e', 'c', 'o', 'g', 'n', 'i', 'z',
					 'e', '_', 'o', 'n', 'l', 'y', '_', 'e',
					 'x', 'e', 'c', 'u', 't', 'a', 'b', 'l',
					 'e', 's', '\0' };
Char STRnohup[]		= { 'n', 'o', 'h', 'u', 'p', '\0' };
Char STRnice[]		= { 'n', 'i', 'c', 'e', '\0' };
Char STRl_pathnum[]	= { 'l', 'i', 's', 't', 'p', 'a', 't', 'h', 'n', 'u', 
			    'm', '\0' };
Char STRshowdots[]	= { 's', 'h', 'o', 'w', 'd', 'o', 't', 's', '\0' };
Char STRthen[]		= { 't', 'h', 'e', 'n', '\0' };
Char STReof[]		= { '^', 'D', '\b', '\b', '\0' };
Char STRl_jobs[]	= { 'l', 'i', 's', 't', 'j', 'o', 'b', 's', '\0' };
Char STRlong[]		= { 'l', 'o', 'n', 'g', '\0' };
Char STRwho[]		= { 'w', 'h', 'o', '\0' };
Char STRrmstar[]        = { 'r', 'm', 's', 't', 'a', 'r', '\0' };
Char STRrm[]            = { 'r', 'm', '\0' };
Char STRshlvl[]		= { 's', 'h', 'l', 'v', 'l', '\0' };
Char STRSHLVL[]		= { 'S', 'H', 'L', 'V', 'L', '\0' };

Char STRLANG[]  = { 'L', 'A', 'N', 'G', '\0' };
Char STRLC_CTYPE[] = { 'L', 'C', '_', 'C', 'T', 'Y', 'P', 'E' ,'\0' };
Char STRNOREBIND[] = { 'N', 'O', 'R', 'E', 'B', 'I', 'N', 'D', '\0' };

#ifdef SIG_WINDOW
Char STRLINES[] = { 'L', 'I', 'N', 'E', 'S', '\0'};
Char STRCOLUMNS[] = { 'C', 'O', 'L', 'U', 'M', 'N', 'S', '\0'};
Char STRTRMCAP[] = { 'T', 'E', 'R', 'M', 'C', 'A', 'P', '\0'};
#endif /* SIG_WINDOW */

#ifdef WARP
Char STRwarp[] = { 'w', 'a', 'r', 'p', '\0' };
#endif /* WARP */

#ifdef apollo
Char STRSYSTYPE[] = { 'S', 'Y', 'S', 'T', 'Y', 'P', 'E', '\0' };
Char STRoid[] = { 'o', 'i', 'd', '\0' };
Char STRbsd43[] = { 'b', 's', 'd', '4', '.', '3', '\0' };
Char STRsys53[] = { 's', 'y', 's', '5', '.', '3', '\0' };
Char STRver[] = { 'v', 'e', 'r', '\0' };
#endif /* apollo */
