#include "../h/param.h"
#include "../h/errno.h"
#include "../h/syslog.h"
#include "../h/uio.h"
#include "ibv.h"

/* Copyright (c) 2021 Aaron Jackson
 *
 * This driver is for the IBV11 IEEE-488 Interface Bus card.
 *
 * Devices can be accessed via /dev/ibv{1..15}. For each of these, a
 * line should be created in /etc/dtab. e.g.
 *
 * ibv 1  160150 420 5 ibvinterr ibvintsr ibvintcmd # IVB11
 * ...
 * ibv 15 160150 420 5 ibvinterr ibvintsr ibvintcmd # IBV11
 *
 * This requires NIDV to be set to 15.
 *
 * The default control register for an IBV11 is 160150 with a default
 * vector at 420.
 */

#define NIBV 1

#define IBV_BUFF_LEN 128
#define IBV_MAX_LOOP 1000
#define IBV_DEBUG 0

/* We will assume there is only one IBV11 card installed */
struct ibvdevice *ibvaddr = (struct ibvdevice *)0160150;

int IBV_OPEN_ADDR = 0; /* which ever remote address is active */
char IBV_CMD_SUCCESSFUL = 0;

ibvattach(addr, unit)
     struct ibvdevice *addr;
     u_int unit;
{
  if (unit <= NIBV) {
    return 1;
  }

  return 0;
}

ibvopen(dev, flag)
     dev_t dev;
     short flag;
{
  if (IBV_OPEN_ADDR > 0)
    return EACCES;

  IBV_CMD_SUCCESSFUL = 0;
  IBV_OPEN_ADDR = minor(dev);

  return 0;
}

ibvclose(dev, flag)
     dev_t	dev;
     int	flag;
{
  IBV_OPEN_ADDR = 0;
  return 0;
}

/* Error interrupt */
ibvinterr(ibv)
     int ibv;
{
  log(LOG_ERR, "ibv%d error interupt", ibv);
}

/* Service request interrupt */
ibvintsr(ibv)
     int ibv;
{
  IBV_CMD_SUCCESSFUL = 1;
#if IBV_DEBUG > 0
  log(LOG_NOTICE, "ibv%d service request interrupt", ibv);
#endif
}

/* Command and talker interrupt */
ibvintcmd(ibv)
     int ibv;
{

#if IBV_DEBUG > 0
  log(LOG_NOTICE, "ibv%d command interrupt", ibv);
#endif
}

/* Listener interrupt */
ibvintlis(ibv)
     int ibv;
{
#if IBV_DEBUG > 0
  log(LOG_NOTICE, "ibv%d listener interrupt", ibv);
#endif
}

ibv_waitintr()
{
  int n = IBV_MAX_LOOP;

  while (!IBV_CMD_SUCCESSFUL) { /* wait for interrupt */
    if (n-- == 0) {
      log(LOG_NOTICE, "ibv: timeout waiting for interrupt");
      return 0;
    }
  }
  IBV_CMD_SUCCESSFUL = 0;
  return 1;
}

ibvread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  struct ibvdevice *ibv;

  if (IBV_OPEN_ADDR != minor(dev))
    return EACCES;

  ibv = ibvaddr;

  /* This needs work... */
  while (ibv->ibs & IBVS_LNR > 0) {
    ibv->ibvcsrl = IBVS_ACC | IBVS_IE | IBVS_LON; /* 320 */
    ibv_waitintr();
    ureadc(ibv->ibvio, uio);
    ibv->ibvio = 0;
    ibv_waitintr();
  }

  return 0;
}

ibvwrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  struct ibvdevice *ibv;
  register int n;
  register char *cp;
  char inbuf[IBV_BUFF_LEN];
  int error;

  if (IBV_OPEN_ADDR != minor(dev))
    return EACCES;

  ibv = ibvaddr;

  /* Clear the bus! Coming through! */
  ibv->ibvcsrl = IBVS_IE | IBVS_IBC; /* 110 */
  ibv_waitintr();

  /* Push start of a frame, which device etc */
  ibv->ibvcsrl = IBVS_IE | IBVS_REM | IBVS_TCS; /* 105 */
  ibv->ibvio = IBV_MSG_UNLISTEN;
  ibv_waitintr();
  ibv->ibvcsrl = IBVS_IE | IBVS_REM | IBVS_TCS; /* 105 */
  ibv->ibvio = IBV_MSG_LISTEN + d;
  ibv_waitintr();

  /* Start pushing the command */
  while (n = MIN(IBV_BUFF_LEN, uio->uio_resid)) {
    cp = inbuf;
    error = uiomove(cp, (int)n, uio);
    if (error)
      return error;
    do {
      ibv->ibvcsrl = IBVS_IE | IBVS_REM | IBVS_TCS; /* 105 */
      ibv->ibvio = *cp++; /* push byte */
      ibv_waitintr();
    } while (--n);
  }

  /* I've finished talking now */
  ibv->ibvcsrl = IBVS_IE | IBVS_REM | IBVS_TCS; /* 105 */
  ibv->ibvio = IBV_MSG_UNTALK;
  ibv_waitintr();

  /* Go back to listening */
  ibv->ibvcsrl = IBVS_ACC | IBVS_IE | IBVS_LON; /* 320 */

  return 0;
}
