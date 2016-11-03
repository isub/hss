/**********************************************************************************************************
 * Software License Agreement (BSD License)                                                               *
 * Author: Norberto R. de Goes Jr.                                                                        *
 *                                                                                                        *
 * Copyright (c) 2011, Norberto R. de Goes Jr..                                                           *
 *                                                                                                        *
 * All rights reserved.                                                                                   *
 *                                                                                                        *
 * Redistribution and use of this software in source and binary forms, with or without modification, are  *
 * permitted provided that the following conditions are met:                                              *
 *                                                                                                        *
 * * Redistributions of source code must retain the above                                                 *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer.                                                                                *
 *                                                                                                        *
 * * Redistributions in binary form must reproduce the above                                              *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer in the documentation and/or other                                               *
 *   materials provided with the distribution.                                                            *
 *                                                                                                        *
 * * Neither the name of the Teraoka Laboratory nor the                                                   *
 *   names of its contributors may be used to endorse or                                                  *
 *   promote products derived from this software without                                                  *
 *   specific prior written permission of Teraoka Laboratory                                              *
 *                                                                                                        *
 *                                                                                                        *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                             *
 *********************************************************************************************************/


/*********************************************************************************************************

 === CpqD/DRC  -  Projeto ADRIMS  -  Mar/2011 ===
 === Dicionario Dx/Cx ===
 Baseado no "dict_sip" do FreeDiameter (www.freediameter.net) 
                                                                                 Norberto R Goes Jr
*********************************************************************************************************/


#include <freeDiameter/extension.h>

#define VENDOR_3GPP_Id  10415
#define APP_CxDx_Id     16777216
#define VENDOR_ETSI_Id  13019

struct dict_object *g_psoDict3GPPVend;  /* 3gpp vendor dictionary object */
struct dict_object *g_psoDictAppCxDx;   /* Application CxDx dictionary object */

struct dict_object *g_psoDictCmdUAR;  /* UAR command dictionary object */
struct dict_object *g_psoDictCmdMAR;  /* MAR command dictionary object */

struct dict_object *g_psoOriginHost;
struct dict_object *g_psoOriginRealm;
struct dict_object *g_psoDestinationHost;
struct dict_object *g_psoDestinationRealm;

/* The content of this file follows the same structure as dict_base_proto.c */

#define CHECK_dict_new( _type, _data, _parent, _ref )      \
  CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

#define CHECK_dict_search( _type, _criteria, _what, _result )    \
  CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );


struct local_rules_definition 
{
  char       *avp_name;
  enum rule_position   position;
  int        min;
  int       max;
};

/*==================================================================*/

#define RULE_ORDER( _position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0 )

/*==================================================================*/

#define PARSE_loc_rules( _rulearray, _parent, _avp_search_flag) {                           \
    size_t __ar;                                                                            \
    for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {             \
      struct dict_rule_data __data = { NULL,                                                \
               (_rulearray)[__ar].position,                                                 \
               0,                                                                           \
               (_rulearray)[__ar].min,                                                      \
               (_rulearray)[__ar].max};                                                     \
      __data.rule_order = RULE_ORDER(__data.rule_position);                                 \
                                                                                            \
      CHECK_FCT(  fd_dict_search(                                                           \
         fd_g_config->cnf_dict,                                                             \
         DICT_AVP,                                                                          \
         _avp_search_flag,                                                                  \
         (_rulearray)[__ar].avp_name,                                                       \
         &__data.rule_avp, 0 ) );                                                           \
      if ( !__data.rule_avp ) {                                                             \
  TRACE_DEBUG(INFO, "AVP Not found: '%s'", (_rulearray)[__ar].avp_name );                   \
  return ENOENT;                                                                            \
      }                                                                                     \
                                                                                            \
      CHECK_FCT_DO( fd_dict_new( fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL), \
        {                                                                                   \
          TRACE_DEBUG(INFO, "Error on rule with AVP '%s'",                                  \
          (_rulearray)[__ar].avp_name );                                                    \
          return EINVAL;                                                                    \
        } );                                                                                \
    }                                                                                       \
  }

#define enumval_def_u32( _val_, _str_ )    \
  { _str_,     { .u32 = _val_ }}

#define enumval_def_os( _len_, _val_, _str_ )        \
  { _str_,     { .os = { .data = (unsigned char *)_val_, .len = _len_ }}}


/*==================================================================*/
/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

int cxdx_dict_init ()
{
  /* load common dictionary objects */
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Origin-Host", &g_psoOriginHost);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Origin-Realm", &g_psoOriginRealm);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Destination-Host", &g_psoDestinationHost);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Destination-Realm", &g_psoDestinationRealm);

  /* 3gpp dictionary object */
  {
    struct dict_vendor_data vendor_data = { VENDOR_3GPP_Id, "3GPP" };
    CHECK_dict_new (DICT_VENDOR, &vendor_data, NULL, &g_psoDict3GPPVend);
  }

  /* Application Cx Dx object */
  {
    struct dict_application_data data  = { APP_CxDx_Id /* NRGJ */, "Diameter CxDx Application"  };
    CHECK_dict_new (DICT_APPLICATION, &data, g_psoDict3GPPVend, &g_psoDictAppCxDx);
  }

  /* ETSI dictionary object */
  {
    struct dict_object *obj;
    struct dict_vendor_data vendor_data = { VENDOR_ETSI_Id, "ETSI" };
    CHECK_dict_new (DICT_VENDOR, &vendor_data, NULL, &obj);
  }

  /* ##### AVP section #################################### */
  {
    struct dict_object * UTF8String_type;
    struct dict_object * DiameterURI_type;

    CHECK_dict_search (DICT_TYPE, TYPE_BY_NAME, "UTF8String",  &UTF8String_type);
    CHECK_dict_search (DICT_TYPE, TYPE_BY_NAME, "DiameterURI", &DiameterURI_type);

    /* Digest AVPs:  */

    /* Visited-Network-Identifier */
    {
      struct dict_avp_data data = 
      { 
        600,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Visited-Network-Identifier",         /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Public-Identity */
    {
      struct dict_avp_data data = 
      { 
        601,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Public-Identity",                    /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Server-Name */
    {
      struct dict_avp_data data = 
      { 
        602,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Server-Name",                        /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new( DICT_AVP, &data, DiameterURI_type/*UTF8String_type*/, NULL);
    }


    /* Optional-Capability */
    {
      struct dict_avp_data data = 
      { 
        605,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Optional-Capability",                /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Feature-List-ID */
    {
      struct dict_avp_data data = 
      { 
        629,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Feature-List-ID",                    /* Name */
        AVP_FLAG_VENDOR,                      /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Feature-List */
    {
      struct dict_avp_data data = 
      { 
        630,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Feature-List",                       /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_MANDATORY,                   /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Server-Capabilities */
    {
      struct dict_object * avp;
      struct dict_avp_data data = 
      { 
        603,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Server-Capabilities",                /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };

      struct local_rules_definition rules[] = 
      { 
        {  "Vendor-Id",       RULE_REQUIRED, -1, 1 },
        {  "Feature-List-ID", RULE_REQUIRED, -1, 1 },
        {  "Feature-List",    RULE_REQUIRED, -1, 1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* User-Authorization-Type */
    {
      struct dict_avp_data data = 
      { 
        623,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "User-Authorization-Type",            /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Supported-Features */
    {
      struct dict_object * avp;
      struct dict_avp_data data = 
      { 
        628,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Supported-Features",                 /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_MANDATORY,                   /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };

      struct local_rules_definition rules[] = 
      { 
        { "Vendor-Id",       RULE_REQUIRED, -1, 1 },
        { "Feature-List-ID", RULE_REQUIRED, -1, 1 },
        { "Feature-List",    RULE_REQUIRED, -1, 1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* UAR-Flags */
    {
      struct dict_avp_data data = 
      { 
        637,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "UAR-Flags",                          /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* SIP-Item-Number */
    {
      struct dict_avp_data data =
      {
        613,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Item-Number",                    /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* SIP-Authentication-Scheme */
    {
      struct dict_avp_data data =
      {
        608,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Authentication-Scheme",          /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* SIP-Authenticate */
    {
      struct dict_avp_data data =
      {
        609,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Authenticate",                   /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* SIP-Authorization */
    {
      struct dict_avp_data data =
      {
        610,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Authorization",                  /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* SIP-Authentication-Context */
    {
      struct dict_avp_data data =
      {
        611,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Authentication-Context",         /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Confidentiality-Key */
    {
      struct dict_avp_data data =
      {
        625,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Confidentiality-Key",                /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Integrity-Key */
    {
      struct dict_avp_data data =
      {
        626,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Integrity-Key",                      /* Name */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
        AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Digest-Realm */
    {
      struct dict_avp_data data =
      {
        104,                  /* Code */
        VENDOR_3GPP_Id,       /* Vendor */
        "Digest-Realm",       /* Name */
        AVP_FLAG_MANDATORY,   /* Fixed flags */
        AVP_FLAG_MANDATORY,   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Digest-Algorithm */
    {
      struct dict_avp_data data =
      {
        111,                  /* Code */
        VENDOR_3GPP_Id,       /* Vendor */
        "Digest-Algorithm",   /* Name */
        AVP_FLAG_MANDATORY,   /* Fixed flags */
        AVP_FLAG_MANDATORY,   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Digest-QoP */
    {
      struct dict_avp_data data =
      {
        110,                  /* Code */
        VENDOR_3GPP_Id,       /* Vendor */
        "Digest-QoP",         /* Name */
        AVP_FLAG_MANDATORY,   /* Fixed flags */
        AVP_FLAG_MANDATORY,   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Digest-HA1 */
    {
      struct dict_avp_data data =
      {
        121,                  /* Code */
        VENDOR_3GPP_Id,       /* Vendor */
        "Digest-HA1",         /* Name */
        AVP_FLAG_MANDATORY,   /* Fixed flags */
        AVP_FLAG_MANDATORY,   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* SIP-Digest-Authenticate */
    {
      struct dict_object * avp;
      struct dict_avp_data data =
      {
        635,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Digest-Authenticate",            /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };
      struct local_rules_definition rules[] = 
      { 
        { "Digest-Realm",     RULE_REQUIRED, -1, 1 },
        { "Digest-Algorithm", RULE_OPTIONAL, -1, 1 },
        { "Digest-QoP",       RULE_REQUIRED, -1, 1 },
        { "Digest-HA1",       RULE_REQUIRED, -1, 1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* Framed-IP-Address */
    {
      struct dict_avp_data data =
      {
        8,                                    /* Code */
        0,                                    /* Vendor */
        "Framed-IP-Address",                  /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_MANDATORY,                   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Framed-IPv6-Prefix */
    {
      struct dict_avp_data data =
      {
        97,                                   /* Code */
        0,                                    /* Vendor */
        "Framed-IPv6-Prefix",                 /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_MANDATORY,                   /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Framed-Interface-Id */
    {
      struct dict_avp_data data =
      {
        96,                                   /* Code */
        0,                                    /* Vendor */
        "Framed-Interface-Id",                /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_MANDATORY,                   /* Fixed flag values */
        AVP_TYPE_UNSIGNED64                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Line-Identifier */
    {
      struct dict_avp_data data =
      {
        500,                                  /* Code */
        13019,                                /* Vendor */
        "Line-Identifier",                    /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* SIP-Auth-Data-Item */
    {
      struct dict_object * avp;
      struct dict_avp_data data =
      {
        612,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Auth-Data-Item",                 /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };
      struct local_rules_definition rules[] = 
      { 
        { "SIP-Item-Number",            RULE_OPTIONAL, -1, 1 },
        { "SIP-Authentication-Scheme",  RULE_OPTIONAL, -1, 1 },
        { "SIP-Authenticate",           RULE_OPTIONAL, -1, 1 },
        { "SIP-Authorization",          RULE_OPTIONAL, -1, 1 },
        { "SIP-Authentication-Context", RULE_OPTIONAL, -1, 1 },
        { "Confidentiality-Key",        RULE_OPTIONAL, -1, 1 },
        { "Integrity-Key",              RULE_OPTIONAL, -1, 1 },
        { "SIP-Digest-Authenticate",    RULE_OPTIONAL, -1, 1 },
        { "Framed-IP-Address",          RULE_OPTIONAL, -1, 1 },
        { "Framed-IPv6-Prefix",         RULE_OPTIONAL, -1, 1 },
        { "Framed-Interface-Id",        RULE_OPTIONAL, -1, 1 },
        { "Line-Identifier",            RULE_OPTIONAL, -1,-1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* SIP-Number-Auth-Items */
    {
      struct dict_avp_data data =
      {
        607,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SIP-Number-Auth-Items",                 /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Wildcarded-Public-Identity */
    {
      struct dict_avp_data data =
      {
        634,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Wildcarded-Public-Identity",         /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
    }

    /* Server-Assignment-Type */
    {
      size_t i;
	    struct dict_object       *type;
	    struct dict_type_data    tdata = { AVP_TYPE_INTEGER32, "Enumerated(Server-Assignment-Type)", NULL, NULL, NULL };
	    struct dict_enumval_data t[] = {
        { "NO_ASSIGNMENT",                            { .i32 =   0 }},
	      { "REGISTRATION",                             { .i32 =   1 }},
	      { "RE_REGISTRATION",                          { .i32 =   2 }},
	      { "UNREGISTERED_USER",                        { .i32 =   3 }},
	      { "TIMEOUT_DEREGISTRATION",                   { .i32 =   4 }},
	      { "USER_DEREGISTRATION",                      { .i32 =   5 }},
	      { "TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME", { .i32 =   6 }},
	      { "USER_DEREGISTRATION_STORE_SERVER_NAME",    { .i32 =   7 }},
	      { "ADMINISTRATIVE_DEREGISTRATION",            { .i32 =   8 }},
	      { "AUTHENTICATION_FAILURE",                   { .i32 =   9 }},
	      { "AUTHENTICATION_TIMEOUT",                   { .i32 =  10 }},
	      { "DEREGISTRATION_TOO_MUCH_DATA",             { .i32 =  11 }},
	      { "AAA_USER_DATA_REQUEST",                    { .i32 =  12 }},
	      { "PGW_UPDATE",                               { .i32 =  13 }},
	      { "RESTORATION",                              { .i32 =  14 }}
      };
      struct dict_avp_data data = {
		    614,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
		    "Server-Assignment-Type",             /* Name */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
		    AVP_TYPE_INTEGER32                    /* base type of data */
	    };
	    /* Create the Enumerated type, and then the AVP */
	    CHECK_dict_new (DICT_TYPE, &tdata , NULL, &type);
      for (i = 0; i < sizeof(t)/sizeof(*t); ++i) {
  	    CHECK_dict_new (DICT_ENUMVAL, &t[i], type, NULL);
      }
	    CHECK_dict_new (DICT_AVP, &data, type, NULL);
    }

    /* User-Data-Already-Available */
    {
      size_t i;
	    struct dict_object       *type;
	    struct dict_type_data    tdata = { AVP_TYPE_INTEGER32, "Enumerated(User-Data-Already-Available)", NULL, NULL, NULL };
	    struct dict_enumval_data t[] = {
        { "USER_DATA_NOT_AVAILABLE",      { .i32 =  0 }},
	      { "USER_DATA_ALREADY_AVAILABLE",  { .i32 =  1 }}
      };
      struct dict_avp_data data = {
		    624,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
		    "User-Data-Already-Available",        /* Name */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
		    AVP_TYPE_INTEGER32                    /* base type of data */
	    };
	    /* Create the Enumerated type, and then the AVP */
	    CHECK_dict_new (DICT_TYPE, &tdata , NULL, &type);
      for (i = 0; i < sizeof(t)/sizeof(*t); ++i) {
  	    CHECK_dict_new (DICT_ENUMVAL, &t[i], type, NULL);
      }
	    CHECK_dict_new (DICT_AVP, &data, type, NULL);
    }

    /* Path */
    {
      struct dict_avp_data data =
      {
        640,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Path",                               /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Contact */
    {
      struct dict_avp_data data =
      {
        641,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Contact",                            /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Initial-CSeq-Sequence-Number */
    {
      struct dict_avp_data data =
      {
        654,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Initial-CSeq-Sequence-Number",       /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Call-ID-SIP-Header */
    {
      struct dict_avp_data data =
      {
        643,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Call-ID-SIP-Header",                 /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* Subscription-Info */
	  {
		  struct dict_avp_data data = {
			  642,	                                /* Code */
			  VENDOR_3GPP_Id,	                      /* Vendor */
			  "Subscription-Info",	                /* Name */
			  AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,  /* Fixed flags */
			  AVP_FLAG_VENDOR,	                    /* Fixed flag values */
			  AVP_TYPE_GROUPED	                    /* base type of data */
		  };
		  CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
	  };

    /* Restoration-Info */
    {
      struct dict_object * avp;
      struct dict_avp_data data =
      {
        649,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "Restoration-Info",                   /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };
      struct local_rules_definition rules[] =
      {
        { "Path",                         RULE_REQUIRED, -1,  1 },
        { "Contact",                      RULE_REQUIRED,  1, -1 },
        { "Initial-CSeq-Sequence-Number", RULE_OPTIONAL, -1,  1 },
        { "Call-ID-SIP-Header",           RULE_OPTIONAL, -1,  1 },
        { "Subscription-Info",            RULE_OPTIONAL, -1,  1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* SCSCF-Restoration-Info */
    {
      struct dict_object * avp;
      struct dict_avp_data data =
      {
        639,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SCSCF-Restoration-Info",             /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_GROUPED                      /* base type of data */
      };
      struct local_rules_definition rules[] = 
      { 
        { "User-Name",                  RULE_REQUIRED, -1,  1 },
        { "Restoration-Info",           RULE_REQUIRED,  1, -1 },
        { "SIP-Authentication-Scheme",  RULE_OPTIONAL, -1,  1 }
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
      PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
    }

    /* Multiple-Registration-Indication */
    {
      size_t i;
	    struct dict_object       *type;
	    struct dict_type_data    tdata = { AVP_TYPE_INTEGER32, "Enumerated(Multiple-Registration-Indication)", NULL, NULL, NULL };
	    struct dict_enumval_data t[] = {
        { "NOT_MULTIPLE_REGISTRATION",  { .i32 =  0 }},
	      { "MULTIPLE_REGISTRATION",      { .i32 =  1 }}
      };
      struct dict_avp_data data = {
		    648,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
		    "Multiple-Registration-Indication",   /* Name */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
		    AVP_TYPE_INTEGER32                    /* base type of data */
	    };
	    /* Create the Enumerated type, and then the AVP */
	    CHECK_dict_new (DICT_TYPE, &tdata , NULL, &type);
      for (i = 0; i < sizeof(t)/sizeof(*t); ++i) {
  	    CHECK_dict_new (DICT_ENUMVAL, &t[i], type, NULL);
      }
	    CHECK_dict_new (DICT_AVP, &data, type, NULL);
    }

    /* Session-Priority */
    {
      size_t i;
	    struct dict_object       *type;
	    struct dict_type_data    tdata = { AVP_TYPE_INTEGER32, "Enumerated(Session-Priority)", NULL, NULL, NULL };
	    struct dict_enumval_data t[] = {
        { "PRIORITY-0", { .i32 =  0 }},
	      { "PRIORITY-1", { .i32 =  1 }},
	      { "PRIORITY-2", { .i32 =  2 }},
	      { "PRIORITY-3", { .i32 =  3 }},
	      { "PRIORITY-4", { .i32 =  4 }}
      };
      struct dict_avp_data data = {
		    650,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
		    "Session-Priority",                   /* Name */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
		    AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flag values */
		    AVP_TYPE_INTEGER32                    /* base type of data */
	    };
	    /* Create the Enumerated type, and then the AVP */
	    CHECK_dict_new (DICT_TYPE, &tdata , NULL, &type);
      for (i = 0; i < sizeof(t)/sizeof(*t); ++i) {
  	    CHECK_dict_new (DICT_ENUMVAL, &t[i], type, NULL);
      }
	    CHECK_dict_new (DICT_AVP, &data, type, NULL);
    }

    /* SAR-Flags */
    {
      struct dict_avp_data data =
      {
        655,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "SAR-Flags",                          /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_UNSIGNED32                   /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }

    /* User-Data */
    {
      struct dict_avp_data data =
      {
        606,                                  /* Code */
        VENDOR_3GPP_Id,                       /* Vendor */
        "User-Data",                          /* Name */
        AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY, /* Fixed flags */
        AVP_FLAG_VENDOR,                      /* Fixed flag values */
        AVP_TYPE_OCTETSTRING                  /* base type of data */
      };
      CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
    }
  } /* end AVP section */

  /* ### Command section ############################# */
  {
    /* User-Authorization-Request (UAR) Command */
    {
      struct dict_cmd_data   data = 
      { 
        300,                                                    /* Code */
        "User-Authorization-Request",                           /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE                   /* Fixed flag values */
      };

      struct local_rules_definition rules[] = 
      {
        { "Session-Id",                     RULE_FIXED_HEAD, -1,  1 },
        { "Vendor-Specific-Application-Id", RULE_REQUIRED,   -1,  1 },
        { "Auth-Session-State",             RULE_REQUIRED,   -1,  1 },
        { "Origin-Host",                    RULE_REQUIRED,   -1,  1 },
        { "Origin-Realm",                   RULE_REQUIRED,   -1,  1 },
        { "Destination-Host",               RULE_OPTIONAL,   -1,  1 },
        { "Destination-Realm",              RULE_REQUIRED,   -1,  1 },
        { "User-Name",                      RULE_REQUIRED,   -1,  1 },
        { "Supported-Features",             RULE_OPTIONAL,   -1, -1 },
        { "Public-Identity",                RULE_REQUIRED,   -1,  1 },
        { "Visited-Network-Identifier",     RULE_REQUIRED,   -1,  1 },
        { "UAR-Flags",                      RULE_OPTIONAL,   -1,  1 },
        { "User-Authorization-Type",        RULE_OPTIONAL,   -1,  1 },
        { "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 },
        { "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &g_psoDictCmdUAR);
      PARSE_loc_rules (rules, g_psoDictCmdUAR, AVP_BY_NAME_ALL_VENDORS);
    }

    /* User-Authorization-Answer (UAA) Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data =
      {
        300,                                                    /* Code */
        "User-Authorization-Answer",                            /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_PROXIABLE                                      /* Fixed flag values */
      };

      struct local_rules_definition rules[] =
      {
        { "Session-Id",                     RULE_FIXED_HEAD,  -1,  1 },
        { "Vendor-Specific-Application-Id", RULE_REQUIRED,    -1,  1 },
        { "Result-Code",                    RULE_OPTIONAL,    -1,  1 },
        { "Experimental-Result-Code",       RULE_OPTIONAL,    -1,  1 },
        { "Auth-Session-State",             RULE_REQUIRED,    -1,  1 },
        { "Origin-Host",                    RULE_REQUIRED,    -1,  1 },
        { "Origin-Realm",                   RULE_REQUIRED,    -1,  1 },
        { "Supported-Features",             RULE_OPTIONAL,    -1, -1 },
        { "Server-Name",                    RULE_OPTIONAL,    -1,  1 },
        { "Server-Capabilities",            RULE_OPTIONAL,    -1,  1 },
        { "Failed-AVP",                     RULE_OPTIONAL,    -1, -1 },
        { "Proxy-Info",                     RULE_OPTIONAL,    -1, -1 },
        { "Route-Record",                   RULE_OPTIONAL,    -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules (rules, cmd, AVP_BY_NAME_ALL_VENDORS);
    }

    /* Multimedia-Auth-Request (MAR) Command */
    {
      struct dict_cmd_data data =
      {
        303,                                                    /* Code */
        "Multimedia-Auth-Request",                              /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE                   /* Fixed flag values */
      };

      struct local_rules_definition rules[] =
      {
        { "Session-Id",                     RULE_FIXED_HEAD, -1,  1 },
        { "Vendor-Specific-Application-Id", RULE_REQUIRED,   -1,  1 },
        { "Auth-Session-State",             RULE_REQUIRED,   -1,  1 },
        { "Origin-Host",                    RULE_REQUIRED,   -1,  1 },
        { "Origin-Realm",                   RULE_REQUIRED,   -1,  1 },
        { "Destination-Realm",              RULE_REQUIRED,   -1,  1 },
        { "Destination-Host",               RULE_OPTIONAL,   -1,  1 },
        { "User-Name",                      RULE_REQUIRED,   -1,  1 },
        { "Supported-Features",             RULE_OPTIONAL,   -1, -1 },
        { "Public-Identity",                RULE_REQUIRED,   -1,  1 },
        { "SIP-Auth-Data-Item",             RULE_REQUIRED,   -1,  1 },
        { "SIP-Number-Auth-Items",          RULE_REQUIRED,   -1,  1 },
        { "Server-Name",                    RULE_REQUIRED,   -1,  1 },
        { "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 },
        { "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &g_psoDictCmdMAR);
      PARSE_loc_rules (rules, g_psoDictCmdMAR, AVP_BY_NAME_ALL_VENDORS);
    }
    /* Multimedia-Auth-Answer (MAA) Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        303,                                                    /* Code */
        "Multimedia-Auth-Answer",                               /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_PROXIABLE                                      /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
      {
        { "Session-Id",                     RULE_FIXED_HEAD, -1,  1 },
        { "Vendor-Specific-Application-Id", RULE_REQUIRED,   -1,  1 },
        { "Result-Code",                    RULE_OPTIONAL,   -1,  1 },
        { "Experimental-Result-Code",       RULE_OPTIONAL,   -1,  1 },
        { "Auth-Session-State",             RULE_REQUIRED,   -1,  1 },
        { "Origin-Host",                    RULE_REQUIRED,   -1,  1 },
        { "Origin-Realm",                   RULE_REQUIRED,   -1,  1 },
        { "User-Name",                      RULE_OPTIONAL,   -1,  1 },
        { "Supported-Features",             RULE_OPTIONAL,   -1, -1 },
        { "Public-Identity",                RULE_OPTIONAL,   -1,  1 },
        { "SIP-Number-Auth-Items",          RULE_OPTIONAL,   -1,  1 },
        { "SIP-Auth-Data-Item",             RULE_OPTIONAL,   -1, -1 },
        { "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 },
        { "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 },
        { "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules (rules, cmd, AVP_BY_NAME_ALL_VENDORS);
    }

    /* Server-Assignment-Request (SAR) Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        301,                                                    /* Code */
        "Server-Assignment-Request",                            /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE                   /* Fixed flag values */
      };
      struct local_rules_definition rules[] =
      {
        { "Session-Id",                       RULE_FIXED_HEAD, -1,  1 },
        { "Vendor-Specific-Application-Id",   RULE_REQUIRED,   -1,  1 },
        { "Auth-Session-State",               RULE_REQUIRED,   -1,  1 },
        { "Origin-Host",                      RULE_REQUIRED,   -1,  1 },
        { "Origin-Realm",                     RULE_REQUIRED,   -1,  1 },
        { "Destination-Host",                 RULE_OPTIONAL,   -1,  1 },
        { "Destination-Realm",                RULE_REQUIRED,   -1,  1 },
        { "User-Name",                        RULE_REQUIRED,   -1,  1 },
        { "Supported-Features",               RULE_OPTIONAL,   -1, -1 },
        { "Public-Identity",                  RULE_REQUIRED,   -1, -1 },
        { "Wildcarded-Public-Identity",       RULE_OPTIONAL,   -1,  1 },
        { "Server-Name",                      RULE_REQUIRED,   -1,  1 },
        { "Server-Assignment-Type",           RULE_REQUIRED,   -1,  1 },
        { "User-Data-Already-Available",      RULE_REQUIRED,   -1,  1 },
        { "SCSCF-Restoration-Info",           RULE_OPTIONAL,   -1,  1 },
        { "Multiple-Registration-Indication", RULE_OPTIONAL,   -1,  1 },
        { "Session-Priority",                 RULE_OPTIONAL,   -1,  1 },
        { "SAR-Flags",                        RULE_OPTIONAL,   -1,  1 },
        { "Proxy-Info",                       RULE_OPTIONAL,   -1, -1 },
        { "Route-Record",                     RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules (rules, cmd, AVP_BY_NAME_ALL_VENDORS);
    }
    /* Server-Assignment-Answer (SAA) Command */
    {
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        301,                                                    /* Code */
        "Server-Assignment-Answer",                             /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
        CMD_FLAG_PROXIABLE                                      /* Fixed flag values */
      };
      struct local_rules_definition rules[] =
      {
        { "Session-Id",       RULE_FIXED_HEAD, -1, 1 },
        { "Vendor-Specific-Application-Id",   RULE_REQUIRED,   -1,  1 },
        { "Result-Code",                      RULE_OPTIONAL,   -1,  1 },
        { "Experimental-Result-Code",         RULE_OPTIONAL,   -1,  1 },
        { "Auth-Session-State",               RULE_REQUIRED,   -1,  1 },
        { "Origin-Host",                      RULE_REQUIRED,   -1,  1 },
        { "Origin-Realm",                     RULE_REQUIRED,   -1,  1 },
        { "User-Name",                        RULE_OPTIONAL,   -1, -1 },
        { "Supported-Features",               RULE_OPTIONAL,   -1, -1 },
        { "User-Data",                        RULE_OPTIONAL,   -1,  1 },
        { "Charging-Information",             RULE_OPTIONAL,   -1,  1 },
        { "Associated-Identities",            RULE_OPTIONAL,   -1,  1 },
        { "Loose-Route-Indication",           RULE_OPTIONAL,   -1,  1 },
        { "SCSCF-Restoration-Info",           RULE_OPTIONAL,   -1, -1 },
        { "Associated-Registered-Identities", RULE_OPTIONAL,   -1,  1 },
        { "Server-Name",                      RULE_OPTIONAL,   -1,  1 },
        { "Wildcarded-Public-Identity",       RULE_OPTIONAL,   -1,  1 },
        { "Priviledged-Sender-Indication",    RULE_OPTIONAL,   -1,  1 },
        { "Failed-AVP",                       RULE_OPTIONAL,   -1, -1 },
        { "Proxy-Info",                       RULE_OPTIONAL,   -1, -1 },
        { "Route-Record",                     RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules (rules, cmd, AVP_BY_NAME_ALL_VENDORS);
    }

#if 0   /* TODO - NRGJ :   alterar conforme RFC-3GPP : */
    /* Location-Info-Request (LIR) Command */
    {
      /*
        The Location-Info-Request (LIR) is indicated by the Command-Code set
        to 285 and the Command Flags' 'R' bit set.  The Diameter client in a
        SIP server sends this command to the Diameter server to request
        routing information, e.g., the URI of the SIP server assigned to the
        SIP-AOR AVP value allocated to the users.

        The Message Format of the LIR command is as follows:

        <LIR> ::= < Diameter Header: 285, REQ, PXY >
        < Session-Id >
        { Auth-Application-Id }
        { Auth-Session-State }
        { Origin-Host }
        { Origin-Realm }
        { Destination-Realm }
        { SIP-AOR }
        [ Destination-Host ]
        * [ Proxy-Info ]
        * [ Route-Record ]
        * [ AVP ]
      */
      struct dict_object * cmd;
      struct dict_cmd_data data = { 
        285,           /* Code */
        "Location-Info-Request",     /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE             /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
      { {  "Session-Id",     RULE_FIXED_HEAD, -1, 1 }
        ,{  "Auth-Application-Id",   RULE_REQUIRED,   -1, 1 }
        ,{  "Auth-Session-State",   RULE_REQUIRED,   -1, 1 }
        ,{  "Origin-Host",     RULE_REQUIRED,   -1, 1 }
        ,{  "Origin-Realm",     RULE_REQUIRED,   -1, 1 }
        ,{  "Destination-Realm",  RULE_REQUIRED,   -1, 1 }
        ,{  "SIP-AOR",       RULE_REQUIRED,   -1, 1 }
        ,{  "Destination-Host",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Proxy-Info",     RULE_OPTIONAL,   -1, -1 }
        ,{  "Route-Record",     RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }
    /* Location-Info-Answer (LIA) Command */
    {
      /*
        The Location-Info-Answer (LIA) is indicated by the Command-Code set
        to 285 and the Command Flags' 'R' bit cleared.  The Diameter server
        sends this command in response to a previously received Diameter
        Location-Info-Request (LIR) command.

        In addition to the values already defined in RFC 3588 [RFC3588], the
        Result-Code AVP may contain one of the values defined in
        Section 10.1.  When the Diameter server finds an error in processing
        the Diameter LIR message, the Diameter server MUST stop the process
        of the message and answer with a Diameter LIA message that includes
        the appropriate error code in the Result-Code AVP value.  When there
        is no error, the Diameter server MUST set the Result-Code AVP value
        to DIAMETER_SUCCESS in the Diameter LIA message.

        One of the errors that the Diameter server may find is that the
        SIP-AOR AVP value is not a valid user in the realm.  In such cases,
        the Diameter server MUST set the Result-Code AVP value to
        DIAMETER_ERROR_USER_UNKNOWN and return it in a Diameter LIA message.

        If the Diameter server cannot process the Diameter LIR command, e.g.,
        due to a database error, the Diameter server MUST set the Result-Code
        AVP value to DIAMETER_UNABLE_TO_COMPLY and return it in a Diameter
        LIA message.  The Diameter server MUST NOT include any SIP-Server-URI
        or SIP-Server-Capabilities AVP in the Diameter LIA message.

        The Diameter server may or may not be aware of a SIP server assigned
        to the SIP-AOR AVP value included in the Diameter LIR message.  If
        the Diameter server is aware of a SIP server allocated to that
        particular user, the Diameter server MUST include the URI of such SIP
        server in the SIP-Server-URI AVP and return it in a Diameter LIA
        message.  This is typically the situation when the user is either
        registered, or unregistered but a SIP server is still assigned to the
        user.

        When the Diameter server is not aware of a SIP server allocated to
        the user (typically the case when the user unregistered), the
        Result-Code AVP value in the Diameter LIA message depends on whether
        the Diameter server is aware that the user has services defined for
        unregistered users:

        o  Those users who have services defined for unregistered users may
        require the allocation of a SIP server to trigger and perhaps
        execute those services.  Therefore, when the Diameter server is
        not aware of an assigned SIP server, but the user has services
        defined for unregistered users, the Diameter server MUST set the
        Result-Code AVP value to DIAMETER_UNREGISTERED_SERVICE and return
        it in a Diameter LIA message.  The Diameter server MAY also
        include a SIP-Server-Capabilities AVP to facilitate the SIP server
        (Diameter client) with the selection of an appropriate SIP server
        with the required capabilities.  Absence of the SIP-Server-
        Capabilities AVP indicates to the SIP server (Diameter client)
        that any SIP server is suitable to be allocated for the user.

        o  Those users who do not have service defined for unregistered users
        do not require further processing.  The Diameter server MUST set
        the Result-Code AVP value to
        DIAMETER_ERROR_IDENTITY_NOT_REGISTERED and return it to the
        Diameter client in a Diameter LIA message.  The SIP server
        (Diameter client) may return the appropriate SIP response (e.g.,
        480 (Temporarily unavailable)) to the original SIP request.

        The Message Format of the LIA command is as follows:

        <LIA> ::= < Diameter Header: 285, PXY >
        < Session-Id >
        { Auth-Application-Id }
        { Result-Code }
        { Auth-Session-State }
        { Origin-Host }
        { Origin-Realm }
        [ SIP-Server-URI ]
        [ SIP-Server-Capabilities ]
        [ Auth-Grace-Period ]
        [ Authorization-Lifetime ]
        [ Redirect-Host ]
        [ Redirect-Host-Usage ]
        [ Redirect-Max-Cache-Time ]
        * [ Proxy-Info ]
        * [ Route-Record ]
        * [ AVP ]
      */
      struct dict_object * cmd;
      struct dict_cmd_data data = {
        285,           /* Code */
        "Location-Info-Answer",     /* Name */
        CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
        CMD_FLAG_PROXIABLE             /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
      { {  "Session-Id",     RULE_FIXED_HEAD, -1, 1 }
        ,{  "Auth-Application-Id",   RULE_REQUIRED,   -1, 1 }
        ,{  "Result-Code",     RULE_REQUIRED,   -1, 1 }
        ,{  "Auth-Session-State",   RULE_REQUIRED,   -1, 1 }
        ,{  "Origin-Host",     RULE_REQUIRED,   -1, 1 }
        ,{  "Origin-Realm",     RULE_REQUIRED,   -1, 1 }
        ,{  "SIP-Server-URI",    RULE_OPTIONAL,   -1, 1 }
        ,{  "SIP-Server-Capabilities",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Auth-Grace-Period",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Authorization-Lifetime",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Redirect-Host",     RULE_OPTIONAL,   -1, 1 }
        ,{  "Redirect-Host-Usage",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Redirect-Max-Cache-Time",   RULE_OPTIONAL,   -1, 1 }
        ,{  "Proxy-Info",     RULE_OPTIONAL,   -1, -1 }
        ,{  "Route-Record",     RULE_OPTIONAL,   -1, -1 }
      };
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }

//#if 0   /* TODO - NRGJ :   alterar conforme RFC-3GPP : */
    /* Registration-Termination-Request (RTR) Command */
    {
      /*
  The Registration-Termination-Request (RTR) command is indicated by
  the Command-Code set to 287 and the Command Flags' 'R' bit set.  The
  Diameter server sends this command to the Diameter client in a SIP
  server to indicate to the SIP server that one or more SIP AORs have
  to be deregistered.  The command allows an operator to
  administratively cancel the registration of a user from a centralized
  Diameter server.

  The Diameter server has the capability to initiate the deregistration
  of a user and inform the SIP server by means of the Diameter RTR
  command.  The Diameter server can decide whether only one SIP AOR is
  going to be deregistered, a list of SIP AORs, or all the SIP AORs
  allocated to the user.

  The absence of a SIP-AOR AVP in the Diameter RTR message indicates
  that all the SIP AORs allocated to the user identified by the
  User-Name AVP are being deregistered.

  The Diameter server MUST include a SIP-Deregistration-Reason AVP
  value to indicate the reason for the deregistration.

  The Message Format of the RTR command is as follows:

  <RTR> ::= < Diameter Header: 287, REQ, PXY >
  < Session-Id >
  { Auth-Application-Id }
  { Auth-Session-State }
  { Origin-Host }
  { Origin-Realm }
  { Destination-Host }
  { SIP-Deregistration-Reason }
  [ Destination-Realm ]
  [ User-Name ]
  * [ SIP-AOR ]
  * [ Proxy-Info ]
  * [ Route-Record ]
  * [ AVP ]

  */

      struct dict_object * cmd;
      struct dict_cmd_data data = 
  { 
    287,           /* Code */
    "Registration-Termination-Request",     /* Name */
    CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
    CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE       /* Fixed flag values */
  };

      struct local_rules_definition rules[] = 
  {  
    { "Session-Id",               RULE_FIXED_HEAD, -1,  1 },
    { "Auth-Application-Id",   RULE_REQUIRED,   -1,  1 },
    { "Auth-Session-State",   RULE_REQUIRED,   -1,  1 },
    { "Origin-Host",     RULE_REQUIRED,   -1,  1 },
    { "Origin-Realm",     RULE_REQUIRED,   -1,  1 },
    { "Destination-Host",   RULE_REQUIRED,   -1,  1 },
    { "SIP-Deregistration-Reason",RULE_REQUIRED,   -1,  1 },  
    { "Destination-Realm",  RULE_OPTIONAL,   -1,  1 },
    { "User-Name",     RULE_OPTIONAL,   -1,  1 },
    { "SIP-AOR",             RULE_REQUIRED,   -1, -1 },
    { "Proxy-Info",     RULE_OPTIONAL,   -1, -1 },
    { "Route-Record",     RULE_OPTIONAL,   -1, -1 }
  };
      
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }
    /* Registration-Termination-Answer (RTA) Command */
    {
      /*
  The Registration-Termination-Answer (RTA) is indicated by the
  Command-Code set to 287 and the Command Flags' 'R' bit cleared.  The
  Diameter client sends this command in response to a previously
  received Diameter Registration-Termination-Request (RTR) command.

  In addition to the values already defined in RFC 3588 [RFC3588], the
  Result-Code AVP may contain one of the values defined in
  Section 10.1.

  If the SIP server (Diameter client) requires a User-Name AVP value to
  process the Diameter RTR request, but the Diameter RTR message did
  not contain a User-Name AVP value, the Diameter client MUST set the
  Result-Code AVP value to DIAMETER_USER_NAME_REQUIRED (see Section
  10.1.2) and return it in a Diameter RTA message.

  The SIP server (Diameter client) applies the administrative
  deregistration to each of the URIs included in each of the SIP-AOR
  AVP values, or, if there is no SIP-AOR AVP present in the Diameter
  RTR request, to all the URIs allocated to the User-Name AVP value.

  The value of the SIP-Deregistration-Reason AVP in the Diameter RTR
  command has an effect on the actions performed at the SIP server
  (Diameter client):

  o  If the value is set to PERMANENT_TERMINATION, then the user has
  terminated his/her registration to the realm.  If informing the
  interested parties (e.g., subscribers to the "reg" event
  [RFC3680]) about the administrative deregistration is supported
  through SIP procedures, the SIP server (Diameter client) will do
  so.  The Diameter Client in the SIP Server SHOULD NOT request a
  new user registration.  The SIP server clears the registration
  state of the deregistered AORs.

  o  If the value is set to NEW_SIP_SERVER_ASSIGNED, the Diameter
  server informs the SIP server (Diameter client) that a new SIP
  server has been allocated to the user, due to some reason.  The
  SIP server, if supported through SIP procedures, will inform the
  interested parties (e.g., subscribers to the "reg" event
  [RFC3680]) about the administrative deregistration at this SIP
  server.  The Diameter client in the SIP server SHOULD NOT request
  a new user registration.  The SIP server clears the registration
  state of the deregistered SIP AORs.

  o  If the value is set to SIP_SERVER_CHANGE, the Diameter server
  informs the SIP server (Diameter client) that a new SIP server has
  to be allocated to the user, e.g., due to user's capabilities
  requiring a new SIP server, or not enough resources in the current
  SIP server.  If informing the interested parties about the
  administrative deregistration is supported through SIP procedures
  (e.g., subscriptions to the "reg" event [RFC3680]), the SIP server
  will do so.  The Diameter client in the SIP Server SHOULD NOT
  request a new user registration.  The SIP server clears the
  registration state of the deregistered SIP AORs.

  o  If the value is set to REMOVE_SIP_SERVER, the Diameter server
  informs the SIP server (Diameter client) that the SIP server will
  no longer be bound in the Diameter server with that user.  The SIP
  server can delete all data related to the user.

  The Message Format of the RTA command is as follows:

  <RTA> ::= < Diameter Header: 287, PXY >
  < Session-Id >
  { Auth-Application-Id }
  { Result-Code }
  { Auth-Session-State }
  { Origin-Host }
  { Origin-Realm }
  [ Authorization-Lifetime ]
  [ Auth-Grace-Period ]
  [ Redirect-Host ]
  [ Redirect-Host-Usage ]
  [ Redirect-Max-Cache-Time ]
  * [ Proxy-Info ]
  * [ Route-Record ]
  * [ AVP ]

  */
      struct dict_object * cmd;
      struct dict_cmd_data data = { 
  287,           /* Code */
  "Registration-Termination-Answer",     /* Name */
  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
  CMD_FLAG_PROXIABLE             /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
  {    {  "Session-Id",     RULE_FIXED_HEAD, -1, 1 }
     ,{  "Auth-Application-Id",   RULE_REQUIRED,   -1, 1 }
     ,{  "Result-Code",     RULE_REQUIRED,   -1, 1 }
     ,{  "Auth-Session-State",   RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Host",     RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Realm",     RULE_REQUIRED,   -1, 1 }
     ,{  "Authorization-Lifetime",  RULE_OPTIONAL,   -1, 1 }
     ,{  "Auth-Grace-Period",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Redirect-Host",     RULE_OPTIONAL,   -1, 1 }
     ,{  "Redirect-Host-Usage",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Redirect-Max-Cache-Time",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Proxy-Info",     RULE_OPTIONAL,   -1, -1 }
     ,{  "Route-Record",     RULE_OPTIONAL,   -1, -1 }

  };
      
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }
    
    /* Push-Profile-Request (PPR) Command */
    {
      /*
  The Push-Profile-Request (PPR) command is indicated by the
  Command-Code set to 288 and the Command Flags' 'R' bit set.  The
  Diameter server sends this command to the Diameter client in a SIP
  server to update either the user profile of an already registered
  user in that SIP server or the SIP accounting information.  This
  allows an operator to modify the data of a user profile or the
  accounting information and push it to the SIP server where the user
  is registered.

  Each user has a user profile associated with him/her and other
  accounting information.  The profile or the accounting information
  may change with time, e.g., due to addition of new services to the
  user.  When the user profile or the accounting information changes,
  the Diameter server sends a Diameter Push-Profile-Request (PPR)
  command to the Diameter client in a SIP server, in order to start
  applying those new services.

  A PPR command MAY contain a SIP-Accounting-Information AVP that
  updates the addresses of the accounting servers.  Changes in the
  addresses of the accounting servers take effect immediately.  The
  Diameter client SHOULD close any existing accounting session with the
  existing server and start providing accounting information to the
  newly acquired accounting server.

  A PPR command MAY contain zero or more SIP-User-Data AVP values
  containing the new user profile.  On selecting the type of user data,
  the Diameter server SHOULD take into account the supported formats at
  the SIP server (SIP-Supported-User-Data-Type AVP sent in a previous
  SAR message) and the local policy.

  The User-Name AVP indicates the user to whom the profile is
  applicable.

  The Message Format of the PPR command is as follows:

  <PPR> ::= < Diameter Header: 288, REQ, PXY >
  < Session-Id >
  { Auth-Application-Id }
  { Auth-Session-State }
  { Origin-Host }
  { Origin-Realm }
  { Destination-Realm }
  { User-Name }
  * [ SIP-User-Data ]
  [ SIP-Accounting-Information ]
  [ Destination-Host ]
  [ Authorization-Lifetime ]
  [ Auth-Grace-Period ]
  * [ Proxy-Info ]
  * [ Route-Record ]
  * [ AVP ]

  */
      struct dict_object * cmd;
      struct dict_cmd_data data = { 
  288,           /* Code */
  "Push-Profile-Request",     /* Name */
  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE             /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
  {    {  "Session-Id",       RULE_FIXED_HEAD, -1, 1 }
     ,{  "Auth-Application-Id",     RULE_REQUIRED,   -1, 1 }
     ,{  "Auth-Session-State",     RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Host",       RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Realm",       RULE_REQUIRED,   -1, 1 }
     ,{  "Destination-Realm",    RULE_REQUIRED,   -1, 1 }
     ,{  "User-Name",       RULE_REQUIRED,   -1, 1 }
     ,{  "SIP-User-Data",       RULE_OPTIONAL,   -1, -1 }
     ,{  "SIP-Accounting-Information",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Destination-Host",     RULE_OPTIONAL,   -1, 1 }
     ,{  "Authorization-Lifetime",     RULE_OPTIONAL,   -1, 1 }
     ,{  "Auth-Grace-Period",     RULE_OPTIONAL,   -1, 1 }
     ,{  "Proxy-Info",       RULE_OPTIONAL,   -1, -1 }
     ,{  "Route-Record",       RULE_OPTIONAL,   -1, -1 }

  };
      
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }
    /* Push-Profile-Answer (PPA) Command */
    {
      /*
      
      
  The Push-Profile-Answer (PPA) is indicated by the Command-Code set to
  288 and the Command Flags' 'R' bit cleared.  The Diameter client
  sends this command in response to a previously received Diameter
  Push-Profile-Request (PPR) command.

  In addition to the values already defined in RFC 3588 [RFC3588], the
  Result-Code AVP may contain one of the values defined in
  Section 10.1.

  If there is no error when processing the received Diameter PPR
  message, the SIP server (Diameter client) MUST download the received
  user profile from the SIP-User-Data AVP values in the Diameter PPR
  message and store it associated with the user specified in the
  User-Name AVP value.

  If the SIP server does not recognize or does not support some of the
  data transferred in the SIP-User-Data AVP values, the Diameter client
  in the SIP server MUST return a Diameter PPA message that includes a
  Result-Code AVP set to the value DIAMETER_ERROR_NOT_SUPPORTED_USER_DATA.

  If the SIP server (Diameter client) receives a Diameter PPR message
  with a User-Name AVP that is unknown, the Diameter client MUST set
  the Result-Code AVP value to DIAMETER_ERROR_USER_UNKNOWN and MUST
  return it to the Diameter server in a Diameter PPA message.

  If the SIP server (Diameter client) receives in the
  SIP-User-Data-Content AVP value (of the grouped SIP-User-Data AVP)
  more data than it can accept, it MUST set the Result-Code AVP value
  to DIAMETER_ERROR_TOO_MUCH_DATA and MUST return it to the Diameter
  server in a Diameter PPA message.  The SIP server MUST NOT override
  the existing user profile with the one received in the PPR message.

  If the Diameter server receives the Result-Code AVP value set to
  DIAMETER_ERROR_TOO_MUCH_DATA in a Diameter PPA message, it SHOULD
  force a new re-registration of the user by sending to the Diameter
  client a Diameter Registration-Termination-Request (RTR) with the
  SIP-Deregistration-Reason AVP value set to SIP_SERVER_CHANGE.  This
  will force a re-registration of the user and will trigger a selection
  of a new SIP server.

  If the Diameter client is not able to honor the command, for any
  other reason, it MUST set the Result-Code AVP value to
  DIAMETER_UNABLE_TO_COMPLY and it MUST return it in a Diameter PPA
  message.

  The Message Format of the PPA command is as follows:

  <PPA> ::= < Diameter Header: 288, PXY >
  < Session-Id >
  { Auth-Application-Id }
  { Result-Code }
  { Auth-Session-State }
  { Origin-Host }
  { Origin-Realm }
  [ Redirect-Host ]
  [ Redirect-Host-Usage ]
  [ Redirect-Max-Cache-Time ]
  * [ Proxy-Info ]
  * [ Route-Record ]
  * [ AVP ]



  */
      struct dict_object * cmd;
      struct dict_cmd_data data = { 
  288,           /* Code */
  "Push-Profile-Answer",     /* Name */
  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,   /* Fixed flags */
  CMD_FLAG_PROXIABLE             /* Fixed flag values */
      };
      struct local_rules_definition rules[] = 
  {    {  "Session-Id",     RULE_FIXED_HEAD, -1, 1 }
     ,{  "Auth-Application-Id",   RULE_REQUIRED,   -1, 1 }
     ,{  "Result-Code",     RULE_REQUIRED,   -1, 1 }
     ,{  "Auth-Session-State",   RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Host",     RULE_REQUIRED,   -1, 1 }
     ,{  "Origin-Realm",     RULE_REQUIRED,   -1, 1 }
     ,{  "Redirect-Host",    RULE_OPTIONAL,   -1, 1 }
     ,{  "Redirect-Host-Usage",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Redirect-Max-Cache-Time",   RULE_OPTIONAL,   -1, 1 }
     ,{  "Proxy-Info",     RULE_OPTIONAL,   -1, -1 }
     ,{  "Route-Record",     RULE_OPTIONAL,   -1, -1 }

  };
      
      CHECK_dict_new( DICT_COMMAND, &data, g_psoDictAppCxDx, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME );
    }

#endif /* TODO - NRGJ */
  }  /* end Command section */

/*  TRACE_DEBUG(INFO, "Extension 'Dictionary CxDx' initialized"); */
  return 0;
}

/*
EXTENSION_ENTRY("dict_cxdx", cxdx_dict_init);
*/
