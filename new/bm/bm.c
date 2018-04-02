#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <strings.h>
#include "bm.h"
#include "Extern.h"
main(argc,argv)
int argc;
char *argv[];
{
	/* test driver for grep based on Boyer-Moore algorithm */
	char BigBuff[MAXBUFF + 3];
	/*
	* We leave one extra character at the beginning and end of the buffer,
	* but don't tell Execute about it. This is so when someone is
	* scanning the buffer and scans past the end (or beginning)
	* we are still technically in the buffer (picky, but there ARE
	* machines which would complain).  We then leave an *additional*
	* free byte at the begining so we can pass an even address to Execute
	* (on some machines, odd address I/O can *COMPLETELY* destroy any
	* speed benefits of bm).
	*/
	int ret = 1, /* return code from Execute */
		NotFound = 0, /* non-zero if file not readable */
		NFiles,
		NPats; /* number of patterns to search for */
	char i,
		*FlagPtr,
		**OptPtr; /* used to scan command line */
	int TextFile /* file to search */;
	struct PattDesc *DescVec[MAXPATS];

	--argc;
	OptPtr = argv + 1;
	while ( argc && **OptPtr == '-') {
		FlagPtr = *OptPtr + 1;
		while (*FlagPtr) {
			switch (*FlagPtr) {
				case 'c': cFlag = 1; break;
				case 'e': eFlag = 1;
					/* get the patterns from next arg */
					NPats = MkDescVec(DescVec,*++OptPtr);
					--argc;
					break;
				case 'f': fFlag = 1; 
					/* read the patterns from a file */
					NPats = GetPatFile(*++OptPtr,DescVec);
					--argc;
					break;
				case 'l': lFlag = 1; break;
				case 'n': nFlag = 1; break;
				case 's': sFlag = 1; break;
				case 'x': xFlag = 1; break;
				case 'h': hFlag = 1; break;
				default:
					fprintf(stderr,
					"bm: invalid option: -%c \n",
					*FlagPtr);
					PutUsage();
					exit(2);
			} /* switch */
			++FlagPtr;
		} /* while */
		++OptPtr; --argc;
	} /* while */
	/* OptPtr now points to patterns */
	if (!fFlag && !eFlag) {
		if (!argc) {
			fprintf(stderr,"bm: no pattern specified\n");
			PutUsage();
			exit(2);
		} else
			NPats = MkDescVec(DescVec,*OptPtr);
		++OptPtr; --argc;
	}
	/* OptPtr now points to first file */
	NFiles = argc;
	if (!NFiles)
		ret &= Execute(DescVec,NPats,0,BigBuff+2);
	else while (argc--) {
		if ((NFiles > 1) || lFlag) FileName = *OptPtr;
		if ((TextFile = open(*OptPtr,O_RDONLY,0)) < 0) {
			fprintf(stderr,"bm: can't open %s\n",*OptPtr);
			NotFound++;
		} else {
			ret &= Execute(DescVec,NPats,TextFile,BigBuff+2);
			if (sFlag && !ret)
				exit(0);
			close(TextFile);
		} /* if */
		++OptPtr;
	} /* while */
	if (cFlag) printf("%d\n",MatchCount);
	exit(NotFound ? 2 : ret);
} /* main */
