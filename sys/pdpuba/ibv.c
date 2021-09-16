/*#include "../h/param.h" */
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

/* We will assume there is only one IBV11 card installed */
struct ibvdevice *ibvaddr = (struct ibvdevice *)0160150;

/* Each IBV Line is an address on the IEEE-488 bus */
struct ibvline ibvlines[NIBV];
struct ibvline *ibvline_active;

ibvattach(addr, unit)
     struct ibvdevice *addr;
     u_int unit;
{
  if (unit <= NIBV) {
    ibvlines[unit].t_addr = (caddr_t)addr;
    return 1;
  }

  return 0;
}

ibvopen(dev, flag)
     dev_t dev;
     short flag;
{
  struct ibvline *line;
  int d; /* minor */

  d = minor(dev);
  line = &ibvlines[d];

  if (!d && !line->t_addr)
    line->t_addr = (caddr_t)ibvaddr; /* default address */

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


}

/* Service request interrupt */
ibvintsr(ibv)
     int ibv;
{
  struct ibvdevice *dev;

  /* Push out the next byte */
  if (ibvbuff_len(ibvline_active->buf->tx) > 0) {
    ibv->ibvio = ibvbuff_pop(buf->tx);
  }
}

/* Command and talker interrupt */
ibvintcmd(ibv)
     int ibv;
{
  struct ibvdevice *dev;
  /* dev = &IBVcsr[ibv]; */


}

/* Begin writing data to the bus.
 * Between each message we have to wait for an interrupt (ibvintsr).
 */
ibvwrite(dev)
  dev_t dev;
{
  struct ibvdevice *ibv;
  struct ibvline *line;
  int d; /* minor device */

  d = minor(dev);
  line = &ibvlines[d];
  ibvline_active = line;
  ibv = (struct ibvdevice *)line->t_addr;

  ibvbuff_push(line->tx, IBV_MSG_UNLISTEN);
  ibvbuff_push(line->tx, IBV_MSG_LISTEN + d);
  /* write data here... */
  ibvbuff_push(line->tx, IBV_MSG_UNTALK);

  /* Set IE, REM, TCS bits of IBS */
  ibv->ibvcsrl = IBVS_IE | IBVS_REM | IBVS_TCS;
  ibv->ibvio = ibvbuff_pop(buf->tx);
}


/* i know i know buffer might get full etc etc pls leave me alone until this is working slightly */
ibvbuff_push(buf,c)
     struct ibvbuff *buf;
     char c;
{
  buf->b[buf->tail++] = c;
}

char ibvbuff_pop(buf)
     struct ibvbuff *buf;
{
  char c;
  c = buf->b[buf->head++];
  if (buf->head == buf->tail)
    buf->head = buf->tail = 0;
  return c;
}

ibvbuff_len(buf)
     struct ibvbuff *buf;
{
  return buf->tail - buf->head;
}
