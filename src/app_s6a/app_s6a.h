#define VENDOR_3GPP_Id  10415
#define APPLICATION_S6A_ID  16777251
#define HSS_HOST  "hss01kzn.epc.mnc027.mcc250.3gppnetwork.org"
#define HSS_RELM  "epc.mnc027.mcc250.3gppnetwork.org"

#define IMSI  "250070700217438"
#define VISITED_PLMN_Id "\52\20\70"

extern struct dict_object *g_psoDict3GPPVend;
extern struct dict_object *g_psoDictAppS6a;
extern struct dict_object *g_psoDictCmdAIR;
extern struct dict_object *g_psoDictAVPSessionId;
extern struct dict_object *g_psoDictAVPAuthApplicationId;
extern struct dict_object *g_psoDictAVPDestHost;
extern struct dict_object *g_psoDictAVPDestRealm;
extern struct dict_object *g_psoDictAVPResultCode;
extern struct dict_object *g_psoDictAVPUserName;
extern struct dict_object *g_psoDictAVPVisitedPLMNId;
extern struct dict_object *g_psoDictAVPAuthSessionState;
extern struct dict_object *g_psoDictAVPRequestedEUTRANAuthenticationInfo;
extern struct dict_object *g_psoDictAVPNumberOfRequestedVectors;
extern struct dict_object *g_psoDictAVPRequestedUTRANGERANAuthenticationInfo;

int app_s6a_init ();

/* функция загрузки объектов словаря */
int app_s6_dict_cache_init ();

/* приложение-клиент s6a */
/* функция инициализации приложения-клиента s6a */
int app_s6a_client_init ();
/* функция инициализации обработчика сигналов */
int signal_handler_init ();
/* функция для отправки сообщения AIR */
int app_s6a_client_send_air (const char *p_pszIMSI);
