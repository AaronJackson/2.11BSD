#include "../h/param.h"
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

/* We will assume there is only one IBV11 card installed */
struct ibvdevice *ibvaddr = (struct ibvdevice *)0160150;

char IBV_CMD_SUCCESSFUL;

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
  int d; /* minor */

  IBV_CMD_SUCCESSFUL = 0;

  d = minor(dev);

  return 0;
}

ibvclose(dev, flag)
     dev_t	dev;
     int	flag;
{
  return 0;
}

/* ****************************************
 * with all interrupts 'know' which device (i.e. card) it comes from
 * as this is passed as an argument, but we don't know which line it
 * comes from. for this reason we keep track of the currently active
 * line in the variable ibvline_active
 * ***************************************
 */

/* Error interrupt */
ibvinterr(ibv)
     int ibv;
{
  struct ibvdevice *dev;
  /* dev = &IBVcsr[ibv]; */

  log(LOG_ERR, "ibv%d error interupt", ibv);
}

/* Service request interrupt */
ibvintsr(ibv)
     int ibv;
{
  IBV_CMD_SUCCESSFUL = 1;
}

/* Command and talker interrupt */
ibvintcmd(ibv)
     int ibv;
{

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
  int d;

  ibv = ibvaddr;
  d = minor(dev);

  while (ibv->ibvds & IBVD_EOI == 0) {
    ibv->ibvcsrl = IBVS_ACC | IBVS_IE | IBVS_LON; /* 320 */
    ibv_waitintr();
    ureadc(ibv->ibvio, uio);
    ibv->ibvio = 0;
    ibv_waitintr();
  }

  return 0;
}


/* Begin writing data to the bus.
 * Between each message we have to wait for an interrupt (ibvintsr).
 */
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
  int d; /* minor device */

  ibv = ibvaddr;
  d = minor(dev);

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
