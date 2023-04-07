#	Makefile	2.0	(2.11BSD) 2020/1/7
#

# if machine has separate I/D
SEPFLAG=-i
VMPGS = 16
# else
# LDFLAGS =
# VMPGS = 4
# endif

CFLAGS=-O -DLIBVMF -I. -DNVMPGS=${VMPGS}

cpp :	cpp.o cpc.o token.o
	$(CC) $(CFLAGS) $(SEPFLAG) -o cpp cpp.o cpc.o token.o -lvmf

clean :
	rm -f *.o cpp

install : cpp
	install -s -m 755 -o root -g wheel cpp $(DESTDIR)/lib
