#include <freeDiameter/extension.h>

#include "app_s6a.h"

int app_s6a_init ()
{
  CHECK_FCT (app_s6_dict_cache_init ());
  CHECK_FCT (app_s6a_client_init ());

  /* Advertise the support for the Gx application in the peer */
	CHECK_FCT (fd_disp_app_support (g_psoDictAppS6a, g_psoDict3GPPVend, 1, 0));

  TRACE_DEBUG(INFO, "s6a application initialized");

  return 0;
}
