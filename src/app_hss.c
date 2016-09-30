#include "app_hss.h"

static int app_hss_entry (const char *p_pszConfFile)
{
  /* suppress compiler warning */
  p_pszConfFile = p_pszConfFile;

  /* регистрация функции валидации пира */
  CHECK_FCT (fd_peer_validate_register (app_pcrf_peer_validate));

  /* инициализация словаря */
  CHECK_FCT (cxdx_dict_init (NULL));

  /* регистрация callback функции обработки ECR запросов */
  CHECK_FCT (app_hss_server_init ());

  /* регистрация приложения */
  CHECK_FCT (fd_disp_app_support (g_psoDictAppCxDx, g_psoDict3GPPVend, 1, 0));

  return 0;
}

void fd_ext_fini (void)
{
  app_hss_server_fini ();
}

EXTENSION_ENTRY ("app_hss", app_hss_entry);
