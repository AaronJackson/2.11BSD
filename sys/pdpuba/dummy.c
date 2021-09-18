
#include "../h/param.h"
#include "../h/syslog.h"
#include "../h/uio.h"
#include "dummy.h"

#define DUMMY_BUFF_LEN 128

dummyattach(addr, unit)
     struct dummy *addr;
     u_int unit;
{
  log(LOG_INFO, "dummy attach");
  return 1;
}

dummyopen(dev, flag)
     dev_t dev;
     short flag;
{
  log(LOG_INFO, "dummy open");
  return 0;
}

dummyclose(dev, flag)
     dev_t dev;
     short flag;
{
  log(LOG_INFO, "dummy close");
  return 0;
}

dummyread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  log(LOG_INFO, "dummy read");
  return 0;
}

dummywrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  register int n;
  register char *cp;
  char inbuf[DUMMY_BUFF_LEN];
  int error;

  while (n = MIN(DUMMY_BUFF_LEN, uio->uio_resid)) {
    cp = inbuf;
    error = uiomove(cp, (int)n, uio);
    if (error)
      return error;
    do
      log(LOG_NOTICE, "dummy write: c=%c", *cp++);
    while (--n);
  }
  return 0;
}
