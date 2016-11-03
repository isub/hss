#include <freeDiameter/extension.h>

#include "app_s6a.h"

struct sess_state {
  os0_t   m_osSid;
  size_t  m_stSidLen;
};

static struct session_handler *g_psoSessHandler = NULL;
static void cleanup_sess_state (struct sess_state *state, os0_t sid, void *opaque);

static void app_s6a_client_aia_cb (void * data, struct msg ** msg);

int app_s6a_client_init ()
{
  CHECK_FCT (signal_handler_init ());
  CHECK_FCT (fd_sess_handler_create (&g_psoSessHandler, cleanup_sess_state, NULL, NULL));

  return 0;
}

int app_s6a_client_send_air (const char *p_pszIMSI)
{
	int iRetVal = 0;
  os0_t sid;
  size_t sidlen;
	struct msg * psoReq = NULL;
	struct avp * psoAVP;
	union avp_value soAVPValue;
	struct sess_state *psoMsgState = NULL;
	struct session *pSess = NULL;

  TRACE_DEBUG (INFO, "Creating a new AIR message for sending");

	/* Create the request */
	CHECK_FCT_DO (fd_msg_new (g_psoDictCmdAIR, MSGFL_ALLOC_ETEID, &psoReq), goto out);
	/* задаем идентификатор приложения */
	{
		struct msg_hdr * psoMsgHdr;
    CHECK_FCT_DO (fd_dict_getval (g_psoDictAppS6a, &psoMsgHdr), goto out);
		CHECK_FCT_DO (fd_msg_hdr (psoReq, &psoMsgHdr), goto out);
		psoMsgHdr->msg_appl = APPLICATION_S6A_ID;
	}

  /* создаем Session-Id */
	{
    CHECK_FCT_DO (fd_sess_new (&pSess, fd_g_config->cnf_diamid, fd_g_config->cnf_diamid_len, NULL, 0), goto out);
	  CHECK_FCT_DO (fd_sess_getsid (pSess, &sid, &sidlen ), goto out );
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPSessionId, 0, &psoAVP), goto out);
		soAVPValue.os.data = sid;
		soAVPValue.os.len = sidlen;
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_FIRST_CHILD, psoAVP), goto out);
	}

	/* Now set all AVPs values */

	/* Set the Auth-Application-Id */
	{
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPAuthApplicationId, 0, &psoAVP), goto out);
		soAVPValue.u32 = APPLICATION_S6A_ID;
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue ), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
	}

	/* Set the Auth-Session-State */
	{
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPAuthSessionState, 0, &psoAVP), goto out);
/*		soAVPValue.u32 = 0; STATE_MAINTAINED */
		soAVPValue.u32 = 1; /* NO_STATE_MAINTAINED */
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue ), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
	}

	/* Set the Destination-Host AVP */
	{
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPDestHost, 0, &psoAVP), goto out);
		soAVPValue.os.data = (uint8_t *) HSS_HOST;
		soAVPValue.os.len  = strlen ((const char*) soAVPValue.os.data);
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
	}

	/* Set the Destination-Realm AVP */
	{
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPDestRealm, 0, &psoAVP), goto out);
		soAVPValue.os.data = (uint8_t *) HSS_RELM;
		soAVPValue.os.len  = strlen ((const char*) soAVPValue.os.data);
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
	}

	/* Set Origin-Host & Origin-Realm */
	CHECK_FCT_DO (fd_msg_add_origin (psoReq, 0), goto out);

  /* set User-Name */
  {
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPUserName, 0, &psoAVP), goto out);
		soAVPValue.os.data = (uint8_t *) IMSI;
		soAVPValue.os.len  = strlen ((const char*) soAVPValue.os.data);
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
  }

  /* Requested-EUTRAN-Authentication-Info */
  //{
  //  struct avp *psoRUAI;
		//CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPRequestedEUTRANAuthenticationInfo, 0, &psoRUAI), goto out);
  //  /* Number-Of-Requested-Vectors */
  //  {
		//  CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPNumberOfRequestedVectors, 0, &psoAVP), goto out);
		//  soAVPValue.u32 = 3;
		//  CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		//  CHECK_FCT_DO (fd_msg_avp_add (psoRUAI, MSG_BRW_LAST_CHILD, psoAVP), goto out);
  //  }
		//CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoRUAI), goto out);
  //}

  /* Requested-UTRAN-GERAN-Authentication-Info */
  {
    struct avp *psoRUAI;
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPRequestedUTRANGERANAuthenticationInfo, 0, &psoRUAI), goto out);
    /* Number-Of-Requested-Vectors */
    {
		  CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPNumberOfRequestedVectors, 0, &psoAVP), goto out);
		  soAVPValue.u32 = 3;
		  CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		  CHECK_FCT_DO (fd_msg_avp_add (psoRUAI, MSG_BRW_LAST_CHILD, psoAVP), goto out);
    }
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoRUAI), goto out);
  }

  /* set Visited-PLMN-Id */
  {
		CHECK_FCT_DO (fd_msg_avp_new (g_psoDictAVPVisitedPLMNId, 0, &psoAVP), goto out);
		soAVPValue.os.data = (uint8_t *) VISITED_PLMN_Id;
		soAVPValue.os.len  = strlen ((const char*) soAVPValue.os.data);
		CHECK_FCT_DO (fd_msg_avp_setvalue (psoAVP, &soAVPValue), goto out);
		CHECK_FCT_DO (fd_msg_avp_add (psoReq, MSG_BRW_LAST_CHILD, psoAVP), goto out);
  }

	/* Store this value in the session */
  CHECK_MALLOC (psoMsgState = malloc (sizeof(struct sess_state)));
  psoMsgState->m_osSid = sid;
  psoMsgState->m_stSidLen = sidlen;
  CHECK_FCT_DO (fd_sess_state_store (g_psoSessHandler, pSess, &psoMsgState), goto out);

	/* Send the request */
	CHECK_FCT_DO (fd_msg_send (&psoReq, app_s6a_client_aia_cb, psoReq), goto out);

out:
	return iRetVal;
}

static void app_s6a_client_aia_cb (void * data, struct msg ** msg)
{
	struct sess_state * mi = NULL;
	struct session *pSess;
	struct avp *pAVP;
	struct avp_hdr *psoHdr;
	int iResultCode;

  /* suppess compiler warning */
  data = data;

	/* Search the session, retrieve its data */
	{
		int iIsNew;
		CHECK_FCT_DO (fd_msg_sess_get (fd_g_config->cnf_dict, *msg, &pSess, &iIsNew), return);
		ASSERT (iIsNew == 0);

		CHECK_FCT_DO (fd_sess_state_retrieve (g_psoSessHandler, pSess, &mi), return);
	}

	/* Value of Result Code */
	CHECK_FCT_DO (fd_msg_search_avp (*msg, g_psoDictAVPResultCode, &pAVP), return);
	if (pAVP) {
		CHECK_FCT_DO (fd_msg_avp_hdr (pAVP, &psoHdr), return);
		iResultCode = psoHdr->avp_value->i32;
	} else {
		iResultCode = -1;
	}

  /* suppress compiler warning */
  iResultCode = iResultCode;

	/* Free the message */
	CHECK_FCT_DO (fd_msg_free (*msg), /*continue*/);
	*msg = NULL;
}

void cleanup_sess_state (struct sess_state *state, os0_t sid, void * opaque)
{
  /* suppress compiler warning */
  sid = sid; opaque = opaque;

  free (state);
}
