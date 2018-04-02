############################################################
############################################################
#####
#####		SENDMAIL CONFIGURATION FILE
#####
#####	This one is the big daddy.  There is no "upstairs"
#####	to bounce a message to -- except perhaps the arpanet.
#####
#####		%W%	%Y%	%G%
#####
############################################################
############################################################



############################################################
###	local info
############################################################

# internet hostname
Cwucbvax vax k UCB-VAX Berkeley UCB-C70 UCB LOCAL

# override hostname to use ARPANET name
DwUCB-VAX

# uucp hostnames
DUucbvax
CUucbvax ernie

# berknet hostname
DBUCBVAX

# UUCP connections on ucbcad
DVucbcad
CVboulder
CVcaddyvax caddyva
CVlblh
CVmasscomp masscom
CVmhb5a
CVmi-cec
CVtekcad
CVjupiter

# known computer center hosts and gateway
include(cchosts.m4)

# we have full sendmail support here
Oa

include(csbase.m4)

include(zerobase.m4)

################################################
###  Machine dependent part of ruleset zero  ###
################################################

# resolve explicit arpanet names (to avoid with machine name "arpa" below)
R$*<@$*$-.ARPA>$*	$#tcp$@$3$:$1<@$2$3.ARPA>$4	user@domain.ARPA

# resolve names that can go via the ethernet
R$*<@$*$=S.$=D>$*	$#ether$@$3$:$1<@$2$3.$4>$5	user@etherhost

# resolve berknet names
R<@$=Z.$=D>:$+		$#berk$@$1$:$2			@berkhost: ...
R$+<@$=Z.$=D>		$#berk$@$2$:$1			user@berknethost

# resolve names destined for the computer center
R$*<@$+.CC>$*		$#ether$@$C$:$1<@$2.CC>$3	user@host.CC
R$*<@$+.BITNET>$*	$#ether$@$C$:$1<@$2.BITNET>$3	user@host.BITNET

# resolve names destined for CSNET
R$*<@$+.CSNET>$*	$#tcp$@CSNET-RELAY$:$1<@$2>$3	user@host.CSNET

# resolve nonlocal UUCP links
R$*<@$*$=V.UUCP>$*	$#ether$@$V$:$1<@$2$3.UUCP>$4	user@host.UUCP
R$*<@$*$=W.UUCP>$*	$#ether$@$W$:$1<@$2$3.UUCP>$4	user@host.UUCP

# resolve local UUCP links (all others)
R<@$+.$-.UUCP>:$+	$#uucp$@$2$:@$1.$2.UUCP:$3	@host.domain.UUCP: ...
R<@$-.UUCP>:$+		$#uucp$@$1$:$2			@host.UUCP: ...
R$+<@$+.$-.UUCP>	$#uucp$@$3$:$1@$2.$3.UUCP	user@host.domain.UUCP
R$+<@$-.UUCP>		$#uucp$@$2$:$1			user@host.UUCP

# resolve mmdf hack
R$+<@UDel-Test>		$#ether$@ucbarpa$:$1@Udel-Test	user.host@UDel-Relay

# other non-local names will be kicked upstairs
R$*<@$*$->$*		$#tcp$@$3$:$1<@$2$3>$4		user@host

# remaining names must be local
R$+			$#local$:$1			everything else

########################################
###  Host dependent address cleanup  ###
########################################

S8
R$*$=U!$+@$+		$3@$4				drop uucp forward


include(berkm.m4)
include(uucpm.m4)
include(tcpm.m4)
