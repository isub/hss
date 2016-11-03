#include "utils/iplistener/iplistener.h"
#include "../app_s6a/app_s6a.h"

#include <stddef.h>

int ps_server_handler (const struct SAcceptedSock *p_psoAcceptedSock);

static struct SIPListener *g_psoIPListener = NULL;

int ps_server_init ()
{
  g_psoIPListener = iplistener_init (
    "0.0.0.0",
    7777,
    32,
    32,
    ps_server_handler);

  if (NULL != g_psoIPListener) {
    return 0;
  } else {
    return -1;
  }
}

void ps_server_fini ()
{
  iplistener_fini (g_psoIPListener);
}

int ps_server_handler (const struct SAcceptedSock *p_psoAcceptedSock)
{
  int iRetVal = 0;

  app_s6a_client_send_air (NULL);

  return iRetVal;
}
