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
