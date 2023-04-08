/*
 *	SCCS id	@(#)NONEboot.s	1.3 (2.11BSD)	2020/1/7
 */
// The intention is for this file to be used if a boot program isn't
// available for a particulare drive/controller, or the autoboot
// feature isn't desired.

halt = 0

.globl _doboot, hardboot
_doboot:
hardboot:
	halt			/ die ...
