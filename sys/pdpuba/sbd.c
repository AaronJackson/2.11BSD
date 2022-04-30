#include "../h/param.h"
#include "../h/errno.h"
#include "../h/syslog.h"
#include "../h/uio.h"
#include "sbd.h"

#define NSBD 1

/*
 * Default status register 0760040
 * Default vector 0270 (PRI 4)
 */

#define dev_t int

struct sbddevice *sbd = (struct sbddevice *)0760040;

sbdattach(addr, unit)
     struct sbddevice *addr;
     u_int unit;
{

  return 1;
}

sbdopen(dev, flag)
     dev_t dev;
     short flag;
{
  static int cmd[] = {
    22,  /* 22 bytes / 11 words */
    (0 << 9) | SBD_FCPIX,
    (0 << 9) | SBD_FCRUB,
    (2 << 9) | SBD_FCALP,
    0300, /* x coord */
    0300, /* y coord */
    06,   /* 6 characters */
    "eh",
    "ll",
    "!o",
    -1
  };

  /* we can probably ignore bits above 16 i think? */
  unsigned int addr = (unsigned int)cmd;

  sbd->hahl = 0000376 & addr;
  sbd->hahr = 0177400 & addr;
  sbd->hcsr = 0;

  return 0;
}

sbdclose(dev, flag)
     dev_t	dev;
     int	flag;
{

  return 0;
}

sbdwrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{

  return 0;
}
