#!/bin/sh -
#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)newvers.sh	1.6 (2.11BSD GTE) 11/26/94
#
if [ ! -r version ]; then echo 0 > version; fi
touch version
echo `cat version` ${USER-root} `pwd` `date` `hostname` | \
awk ' {
	version = $1 + 1; user = $2; host = $10; dir = $3; \
	date = $4 " " $5 " " $6 " " $7 " " $8 " " $9;
}\
END {
	printf "char version[] = \"2.11 BSD UNIX #%d: %s\\n", \
		version, date; \
	printf "    %s@%s:%s\\n\";\n", user, host, dir;
	printf "%d\n", version > "version";
}' > vers.c
