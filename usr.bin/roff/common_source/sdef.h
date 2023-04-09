/*	sdef.h	4.3	(2.11BSD) 2020/3/24 */

struct s {
	int nargs;
	struct s *pframe;
	int pipb;
	int pipx;
	int pnchar;
	int prchar;
	int ppendt;
	int *pap;
	int *pcp;
	int pch0;
	int pch;
	};
