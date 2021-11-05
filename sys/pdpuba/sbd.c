#include "../h/param.h"
#include "../h/errno.h"
#include "../h/syslog.h"
#include "../h/uio.h"
#include "sbd.h"

#define NSBD 1

/*
 * Default status register 0760010
 * Default vector 0270 (PRI 4)
 */

struct sbddevice *sbd = (struct sbddevice *)0760010;

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
