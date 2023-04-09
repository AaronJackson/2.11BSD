/* 
 * Structure for the wchan table
 */
typedef struct wc_s
{
        char		cname[8];
        unsigned        caddr;
} wc_t;
/*
 * structure for the terminal table
 *  store 14 chars of name as MAXNAMLEN uses too much memory
 *  and device names tend to be short
 */
typedef struct  ttab_s
{
        char    name[14];
        dev_t   ttyd;
} ttab_t;

extern u_int	proc_sym;
extern u_int	nproc_sym;
extern u_int	hz_sym;

extern int	hz;
extern u_int	nproc;

extern int	nttys;
extern int	nchans;
extern wc_t 	*wclist;
extern ttab_t	*ttlist;

int getksyms();
