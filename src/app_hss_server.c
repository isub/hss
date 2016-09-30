#include "app_hss.h"

/* данные запроса */
/* Terminal-Information */
struct STerminalInformation {
  struct octet_string m_soIMEI;
  struct octet_string m_soSoftwareVersion;
  struct octet_string m_so3GPP2MEID;
};
/* ECR data */
struct SECRData {
  int m_iAuthSessionState;
  struct octet_string m_soOriginHost;
  struct octet_string m_soOriginRealm;
  struct STerminalInformation m_soTerminalInformation;
};
/* очистка */
void app_hss_clean_req_data (struct SECRData *p_psoECRData);
void app_hss_clean_os (struct octet_string *p_psoOS);
void app_hss_os_copy (struct octet_string *p_psoDst, unsigned char *p_pData, int p_iLen);

/* выборка данных из запроса */
int pcrf_extract_req_data (msg_or_avp *p_psoMsgOrAVP, struct SECRData *p_psoECRData);
int pcrf_extract_terminal_information (struct avp *p_psoAVP, struct STerminalInformation *p_psoTerminalInformation);

/* хэндлер для обработчика ECR запросов */
static struct disp_hdl *g_pCBHandler = NULL;

/* callback-функция для обработки */
int app_hss_uar_cb (struct msg **, struct avp *, struct session *, void *, enum disp_action *);

int app_hss_server_init ()
{
  int iRetVal = 0;
  struct disp_when data;

  TRACE_DEBUG (FULL, "Initializing dispatch callbacks for ECR");

  memset (&data, 0, sizeof (data));
  data.app = g_psoDictAppCxDx;
  data.command = g_psoDictCmdUAR;

  /* Now specific handler for ECR */
  CHECK_FCT (fd_disp_register (app_hss_uar_cb, DISP_HOW_CC, &data, NULL, &g_pCBHandler));

  return iRetVal;
}

void app_hss_server_fini (void)
{
  if (g_pCBHandler) {
    (void)fd_disp_unregister (&g_pCBHandler, NULL);
  }
}

int app_hss_uar_cb (struct msg **p_ppMsg, struct avp *p_pAVP, struct session *p_pSess, void *p_pOpaque, enum disp_action *p_pAct)
{
  struct msg *pAns = NULL;
  struct SECRData soECRData;

  /* подавяем предупреждение компилятора о неиспользуемом параметре */
  p_pAVP = p_pAVP;
  p_pSess = p_pSess;
  p_pOpaque = p_pOpaque;
  p_pAct = p_pAct;

  /* выборка данных из запроса */
  memset (&soECRData, 0, sizeof (soECRData));
  pcrf_extract_req_data (*p_ppMsg, &soECRData);

  /* формирование ответа */
  CHECK_FCT_DO (fd_msg_new_answer_from_req (fd_g_config->cnf_dict, p_ppMsg, 0), goto cleanup_and_exit);
  pAns = *p_ppMsg;

  /* Set the Origin-Host, Origin-Realm, Result-Code AVPs */
  CHECK_FCT_DO (fd_msg_rescode_set (pAns, (char *)"DIAMETER_SUCCESS", NULL, NULL, 1), goto cleanup_and_exit);

  /* Destination-Realm */
  {
    union avp_value soAVPVal;
    struct avp *psoChildAVP = NULL;
    CHECK_FCT_DO (fd_msg_avp_new (g_psoDestinationRealm, 0, &psoChildAVP), goto cleanup_and_exit);
    soAVPVal.os.data = soECRData.m_soOriginRealm.data;
    soAVPVal.os.len = soECRData.m_soOriginRealm.len;
    CHECK_FCT_DO (fd_msg_avp_setvalue (psoChildAVP, &soAVPVal), goto cleanup_and_exit);
    CHECK_FCT_DO (fd_msg_avp_add (pAns, MSG_BRW_LAST_CHILD, psoChildAVP), goto cleanup_and_exit);
  }
  /* Destination-Host */
  {
    union avp_value soAVPVal;
    struct avp *psoChildAVP = NULL;
    CHECK_FCT_DO (fd_msg_avp_new (g_psoDestinationHost, 0, &psoChildAVP), goto cleanup_and_exit);
    soAVPVal.os.data = soECRData.m_soOriginRealm.data;
    soAVPVal.os.len = soECRData.m_soOriginRealm.len;
    CHECK_FCT_DO (fd_msg_avp_setvalue (psoChildAVP, &soAVPVal), goto cleanup_and_exit);
    CHECK_FCT_DO (fd_msg_avp_add (pAns, MSG_BRW_LAST_CHILD, psoChildAVP), goto cleanup_and_exit);
  }

  if (pAns) {
    CHECK_FCT_DO (fd_msg_send (p_ppMsg, NULL, NULL), /*continue*/);
  }

cleanup_and_exit:
  app_hss_clean_req_data (&soECRData);

  return 0;
}

int pcrf_extract_req_data (msg_or_avp *p_psoMsgOrAVP, struct SECRData *p_psoECRData)
{
  int iRetVal = 0;

  struct avp *psoAVP;
  struct avp_hdr *psoAVPHdr;
  vendor_id_t tVenId;

  /* ищем первую AVP */
  iRetVal = fd_msg_browse_internal (p_psoMsgOrAVP, MSG_BRW_FIRST_CHILD, (void **)&psoAVP, NULL);
  if (iRetVal) {
    return iRetVal;
  }

  do {
    /* получаем заголовок AVP */
    if (NULL == psoAVP)
      break;
    iRetVal = fd_msg_avp_hdr (psoAVP, &psoAVPHdr);
    if (iRetVal) {
      break;
    }
    if (AVP_FLAG_VENDOR & psoAVPHdr->avp_flags) {
      tVenId = psoAVPHdr->avp_vendor;
    } else {
      tVenId = (vendor_id_t)-1;
    }
    switch (tVenId) {
    case (vendor_id_t)-1: /* vendor undefined */
    case 0: /* Diameter */
      switch (psoAVPHdr->avp_code) {
      case 264: /* Origin-Host */
        app_hss_os_copy (&p_psoECRData->m_soOriginHost, psoAVPHdr->avp_value->os.data, psoAVPHdr->avp_value->os.len);
        break;
      case 277: /* Auth-Session-State */
        p_psoECRData->m_iAuthSessionState = psoAVPHdr->avp_value->i32;
        break;
      case 296: /* Origin-Realm */
        app_hss_os_copy (&p_psoECRData->m_soOriginRealm, psoAVPHdr->avp_value->os.data, psoAVPHdr->avp_value->os.len);
        break;
      }
      break; /* Diameter */ /* vendor undefined */
    case 10415: /* 3GPP */
      switch (psoAVPHdr->avp_code) {
      case 1401: /* Terminal-Information */
        pcrf_extract_terminal_information (psoAVP, &p_psoECRData->m_soTerminalInformation);
        break;
      }
      break; /* 3GPP */
    }
  } while (0 == fd_msg_browse_internal ((void *)psoAVP, MSG_BRW_NEXT, (void **)&psoAVP, NULL));

  return iRetVal;
}

int pcrf_extract_terminal_information (struct avp *p_psoAVP, struct STerminalInformation *p_psoTerminalInformation)
{
  int iRetVal = 0;

  struct avp *psoAVP;
  struct avp_hdr *psoAVPHdr;

  iRetVal = fd_msg_browse_internal ((void *)p_psoAVP, MSG_BRW_FIRST_CHILD, (void **)&psoAVP, NULL);
  if (iRetVal) {
    return iRetVal;
  }

  do {
    /* получаем заголовок AVP */
    if (NULL == psoAVP)
      break;
    iRetVal = fd_msg_avp_hdr (psoAVP, &psoAVPHdr);
    if (iRetVal) {
      break;
    }
    switch (psoAVPHdr->avp_vendor) {
    case 10415:
      switch (psoAVPHdr->avp_code) {
      case 1402: /* IMEI */
        app_hss_os_copy (&p_psoTerminalInformation->m_soIMEI, psoAVPHdr->avp_value->os.data, psoAVPHdr->avp_value->os.len);
        break;
      case 1403: /* Software-Version */
        app_hss_os_copy (&p_psoTerminalInformation->m_soSoftwareVersion, psoAVPHdr->avp_value->os.data, psoAVPHdr->avp_value->os.len);
        break;
      case 1471: /* 3GPP2-MEID */
        app_hss_os_copy (&p_psoTerminalInformation->m_so3GPP2MEID, psoAVPHdr->avp_value->os.data, psoAVPHdr->avp_value->os.len);
        break;
      }
      break;
    default:
      break;
    }
  } while (0 == fd_msg_browse_internal ((void *)psoAVP, MSG_BRW_NEXT, (void **)&psoAVP, NULL));

  return iRetVal;
}

void app_hss_clean_req_data (struct SECRData *p_psoECRData)
{
  app_hss_clean_os (&p_psoECRData->m_soOriginHost);
  app_hss_clean_os (&p_psoECRData->m_soOriginRealm);
  app_hss_clean_os (&p_psoECRData->m_soTerminalInformation.m_so3GPP2MEID);
  app_hss_clean_os (&p_psoECRData->m_soTerminalInformation.m_soIMEI);
  app_hss_clean_os (&p_psoECRData->m_soTerminalInformation.m_soSoftwareVersion);
}

void app_hss_clean_os (struct octet_string *p_psoOS)
{
  if (p_psoOS->data) {
    free (p_psoOS->data);
    p_psoOS->data = NULL;
    p_psoOS->len = 0;
  }
}

void app_hss_os_copy (struct octet_string *p_psoDst, unsigned char *p_pData, int p_iLen)
{
  p_psoDst->data = malloc (p_iLen);
  if (p_psoDst->data) {
    memcpy (p_psoDst->data, p_pData, p_iLen);
    p_psoDst->len = p_iLen;
  }
}

int app_pcrf_peer_validate (struct peer_info *p_psoPeerInfo, int *p_piAuth, int (**cb2)(struct peer_info *))
{
  int iRetVal = 0;

  /* подавяем предупреждение компилятора о неиспользуемом параметре */
  cb2 = cb2;

  /* проверку пира необходимо провести по следующим параемтрам: */
  p_psoPeerInfo->pi_diamid = p_psoPeerInfo->pi_diamid;
  p_psoPeerInfo->pi_diamidlen = p_psoPeerInfo->pi_diamidlen;
  /**/

  /* если проверка пройдена успешно */
  p_psoPeerInfo->config.pic_flags.sec = PI_SEC_NONE;
  *p_piAuth = 1;

  return iRetVal;
}
