#include <freeDiameter/freeDiameter-host.h>
#include <freeDiameter/libfdproto.h>
#include <freeDiameter/extension.h>

struct octet_string {
  unsigned char *data;  /* bytes buffer */
  size_t          len;  /* length of the data buffer */
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

/* инициализация словаря */
int cxdx_dict_init ();
/* инициализация обработчика ECR команд */
int app_hss_server_init ();
/* выгрузка сервера */
void app_hss_server_fini (void);

/* валидация клиента */
int app_pcrf_peer_validate (struct peer_info *p_psoPeerInfo, int *p_piAuth, int (**cb2)(struct peer_info *));

/* кешированные объекты словаря */
extern struct dict_object *g_psoDict3GPPVend; /* 3gpp vendor dictionary object */
extern struct dict_object *g_psoDictAppCxDx;  /* Application CxDx dictionary object */
extern struct dict_object *g_psoDictCmdUAR;   /* UAR command dictionary object */
extern struct dict_object *g_psoDictCmdMAR;   /* MAR command dictionary object */

extern struct dict_object *g_psoOriginHost;
extern struct dict_object *g_psoOriginRealm;
extern struct dict_object *g_psoDestinationHost;
extern struct dict_object *g_psoDestinationRealm;
