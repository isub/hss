#include <freeDiameter/extension.h>

#include "app_hss.h"
#include "ps_server/ps_server.h"

int app_hss_init (char * conffile)
{
  /* suppress compiler warning */
  conffile = conffile;

  /* инициализация приложения S6a */
  CHECK_FCT (app_s6a_init());

  /* инициализация сервера сообщений PS */
  CHECK_FCT (ps_server_init ());

  TRACE_DEBUG(INFO, "hss application initialized");
  return 0;
}
EXTENSION_ENTRY("app_hss", app_hss_init, "dict_s6a");
