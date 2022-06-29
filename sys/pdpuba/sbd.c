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

/* Simple example to set to graphics mode, erase screen and write hello */
static int sbdcmd[] = {
  22,  /* 22 bytes / 11 words */
  (0 << 8) | SBD_FCPIX,
  (0 << 8) | SBD_FCRUB,
  (4 << 8) | SBD_FCALP,
  0300, /* x coord */
  0300, /* y coord */
  06,   /* 6 characters */
  "eh",
  "ll",
  "!o",
  -1
};

sbdopen(dev, flag)
     dev_t dev;
     short flag;
{

  /* Since the command block will remain in kernel space (i.e. lower
   * addresses), we can ignore the most significant bits of the
   * command block address. This will avoid needing to handle pages
   * etc.
   */
  unsigned int addr = (unsigned int)sbdcmd;

  /* 0376 to avoid overwriting Q (abort) - we are guarenteed a word
   * aligned memory block anyway.
   */
  sbd->hahl = 0376 & addr;
  sbd->hahr = 0377 & (addr >> 8);
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
