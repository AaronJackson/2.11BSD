"/*	@(#)gram	2.3	SCCS id keyword	*/
/yyval/s//*&/
/\*yysterm\[]/,$d
1;/yyactr/ka
'a,$s/yypv/yyYpv/g
'aa
	register int **yyYpv;
	register int *p, *q;
	yyYpv = yypv;
.
1;/^##/-w! y.tab.h
/^int yylval = 0/d
/extern int yychar,/s//extern/
/yyclearin/d
/yyerrok/d
1;/^##/d
$a

yyEactr(__np__, var)
int __np__;
char *var;
{
switch(__np__) {
default:
return (1);
.
g/case.*@@/s/@@//\
.m$
g/@@/ka\
'a;?case?,?case?t$\
'am$\
a\
}\
break;
$a
}
}
.
1,$s/@@//
/int nterms/d
/int nnonter/d
/int nstate/d
/int yyerrval/d
w!
q
