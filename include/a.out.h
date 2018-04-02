/*-
 * Copyright (c) 1991 The Regents of the University of California.
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
 *
 *	@(#)a.out.h	5.6.1 (2.11BSD GTE) 1/6/94
 */

#ifndef	_AOUT_H_
#define	_AOUT_H_

#include <sys/exec.h>

#define	N_BADMAG(x) \
	(((x).a_magic)!=A_MAGIC1 && ((x).a_magic)!=A_MAGIC2 && \
	((x).a_magic)!=A_MAGIC3 && ((x).a_magic)!=A_MAGIC4 && \
	((x).a_magic)!=A_MAGIC5 && ((x).a_magic)!=A_MAGIC6)

#define	N_TXTOFF(x) \
	((x).a_magic==A_MAGIC5 || (x).a_magic==A_MAGIC6 ? \
	sizeof(struct ovlhdr) + sizeof(struct exec) : sizeof(struct exec))

/*
 * The following were added as part of the new object file format.  They
 * call functions because calculating the sums of overlay sizes was too
 * messy (and verbose) to do 'inline'.
 *
 * NOTE: if the magic number is that of an overlaid object the program
 * must pass an extended header ('xexec') as the argument.
*/

#include <sys/types.h>

off_t	n_stroff(), n_symoff(), n_datoff(), n_dreloc(), n_treloc();

#define	N_STROFF(e) (n_stroff(&e))
#define	N_SYMOFF(e) (n_symoff(&e))
#define	N_DATOFF(e) (n_datoff(&e))
#define	N_DRELOC(e) (n_dreloc(&e))
#define	N_TRELOC(e) (n_treloc(&e))

#define	_AOUT_INCLUDE_
#include <nlist.h>

#endif	/* !_AOUT_H_ */
