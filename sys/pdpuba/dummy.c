
#include "../h/param.h"
#include "../h/syslog.h"
#include "dummy.h"

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
  return 1;
}

dummyclose(dev, flag)
     dev_t dev;
     short flag;
{
  log(LOG_INFO, "dummy close");
  return 1;
}

dummyread(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  log(LOG_INFO, "dummy read");
  return 1;
}

dummywrite(dev, uio, flag)
     dev_t dev;
     struct uio *uio;
     int flag;
{
  log(LOG_INFO, "dummy write");
  return 1;
}
