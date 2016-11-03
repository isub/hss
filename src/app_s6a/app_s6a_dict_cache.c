#include <freeDiameter/extension.h>

#include "app_s6a.h"

struct dict_object *g_psoDict3GPPVend;
struct dict_object *g_psoDictAppS6a;

struct dict_object *g_psoDictCmdAIR;
struct dict_object *g_psoDictAVPSessionId;
struct dict_object *g_psoDictAVPAuthApplicationId;
struct dict_object *g_psoDictAVPDestHost;
struct dict_object *g_psoDictAVPDestRealm;
struct dict_object *g_psoDictAVPResultCode;
struct dict_object *g_psoDictAVPUserName;
struct dict_object *g_psoDictAVPVisitedPLMNId;
struct dict_object *g_psoDictAVPAuthSessionState;
struct dict_object *g_psoDictAVPRequestedEUTRANAuthenticationInfo;
struct dict_object *g_psoDictAVPNumberOfRequestedVectors;
struct dict_object *g_psoDictAVPRequestedUTRANGERANAuthenticationInfo;

int app_s6_dict_cache_init ()
{
  /* загрузка объекта словаря вендора */
  {
    vendor_id_t vendorid = VENDOR_3GPP_Id;
    CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_VENDOR, VENDOR_BY_ID, &vendorid, &g_psoDict3GPPVend, ENOENT));
  }

  /* загрузка объекта словаря приложения */
  {
    application_id_t app_id = APPLICATION_S6A_ID;
    CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_APPLICATION, APPLICATION_BY_ID, &app_id, &g_psoDictAppS6a, ENOENT));
  }

  /* команды */
  /* загрузка объекта словаря команды AIR */
   CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_COMMAND, CMD_BY_NAME, "Authentication-Information-Request", &g_psoDictCmdAIR, ENOENT));

  /* AVP */
  /* загрузка объекта словаря AVP Session-Id */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Session-Id", &g_psoDictAVPSessionId, ENOENT));
  /* загрузка объекта словара AVP Auth-Application-Id */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Auth-Application-Id", &g_psoDictAVPAuthApplicationId, ENOENT));
  /* загрузка объекта словара AVP Auth-Session-State */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Auth-Session-State", &g_psoDictAVPAuthSessionState, ENOENT));
  /* загрузка объекта словаря AVP Destination-Host */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Destination-Host", &g_psoDictAVPDestHost, ENOENT));
  /* загрузка объекта словаря AVP Destination-Realm */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Destination-Realm", &g_psoDictAVPDestRealm, ENOENT));
  /* загрузка объекта словаря AVP User-Name */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "User-Name", &g_psoDictAVPUserName, ENOENT));
  /* загрузка объекта словаря AVP Result-Code */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Result-Code", &g_psoDictAVPResultCode, ENOENT));
  /* загрузка объекта словаря AVP Visited-PLMN-Id */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Visited-PLMN-Id", &g_psoDictAVPVisitedPLMNId, ENOENT));
  /* загрузка объекта словаря AVP Requested-EUTRAN-Authentication-Info */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Requested-EUTRAN-Authentication-Info", &g_psoDictAVPRequestedEUTRANAuthenticationInfo, ENOENT));
  /* загрузка объекта словаря AVP Requested-UTRAN-GERAN-Authentication-Info */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Requested-UTRAN-GERAN-Authentication-Info", &g_psoDictAVPRequestedUTRANGERANAuthenticationInfo, ENOENT));
  /* загрузка объекта словаря AVP Number-Of-Requested-Vectors */
  CHECK_FCT (fd_dict_search (fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Number-Of-Requested-Vectors", &g_psoDictAVPNumberOfRequestedVectors, ENOENT));

  return 0;
}
