#ifndef _STDARG_H_
#define _STDARG_H_

/* 
 * stdarg.h  1.0  (2.11BSD)  2020/1/4
*/

typedef char *va_list;
#define va_start(list, fmt) (list = ((char *)&fmt  + sizeof(fmt)))
#define va_end(list)
#define va_arg(list,mode) ((mode *)(list += sizeof(mode)))[-1]

#endif
