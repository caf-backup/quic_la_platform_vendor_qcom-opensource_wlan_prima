/*============================================================================
  FILE:         vos_nvitem.c
  OVERVIEW:     This source file contains definitions for vOS NV Item APIs
  DEPENDENCIES: NV, remote API client, WinCE REX
                Copyright (c) 2008 QUALCOMM Incorporated.
                All Rights Reserved.
                Qualcomm Confidential and Proprietary
============================================================================*/
/*============================================================================
  EDIT HISTORY FOR MODULE
============================================================================*/
// the following is used to disable warning for having too many labels in
// the 'nv_items_enum_type'

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "vos_types.h"
#include "vos_nvitem.h"
#include "vos_trace.h"
#include "vos_api.h"
#include "wlan_hdd_misc.h"
#include "vos_sched.h"
#include "halNv.h"
//#include <wlan_nlink_common.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define VALIDITY_BITMAP_NV_ID    NV_WLAN_VALIDITY_BITMAP_I
#define VALIDITY_BITMAP_SIZE     32
#define MAX_COUNTRY_COUNT        300
//To be removed when NV support is fully functional
#define VOS_HARD_CODED_MAC    {0, 0x0a, 0xf5, 4, 5, 6}
/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
// this wrapper structure is identical to nv_cmd_type except the
// data_ptr type is changed void* to avoid exceeding the debug information
// module size as there are too many elements within nv_items_type union

// structure for code and regulatory domain of a single country
typedef struct
{
   v_U8_t            regDomain;
   v_COUNTRYCODE_t   countryCode;
} CountryInfo_t;
// structure of table to map country code and regulatory domain
typedef struct
{
   v_U16_t           countryCount;
   CountryInfo_t     countryInfo[MAX_COUNTRY_COUNT];
} CountryInfoTable_t;
/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
// cache of country info table;
// this is re-initialized from data on binary file
// loaded on driver initialization if available
static CountryInfoTable_t countryInfoTable =
{
    254,
    {
        { REGDOMAIN_FCC, {'U', 'S'}},       // USA - must be the first country code
        { REGDOMAIN_WORLD, {'A', 'D'}},     //ANDORRA
        { REGDOMAIN_WORLD,{'A', 'E'}},   //UAE
        { REGDOMAIN_WORLD, {'A', 'F'}},     //AFGHANISTAN
        { REGDOMAIN_NO_5GHZ, {'A', 'G'}},     //ANTIGUA AND BARBUDA
        { REGDOMAIN_NO_5GHZ, {'A', 'I'}},     //ANGUILLA
        { REGDOMAIN_WORLD, {'A', 'L'}},     //ALBANIA
        { REGDOMAIN_NO_5GHZ, {'A', 'M'}},     //ARMENIA
        { REGDOMAIN_WORLD, { 'A', 'N'}},     //NETHERLANDS ANTILLES
        { REGDOMAIN_NO_5GHZ, { 'A', 'O'}},     //ANGOLA
        { REGDOMAIN_WORLD, { 'A', 'Q'}},     //ANTARCTICA
        { REGDOMAIN_HI_5GHZ,{ 'A', 'R'}},   //ARGENTINA
        { REGDOMAIN_FCC, { 'A', 'S'}},     //AMERICAN SOMOA
        { REGDOMAIN_ETSI, { 'A', 'T'}},      //AUSTRIA
        { REGDOMAIN_ETSI, { 'A', 'U'}},      //AUSTRALIA
        { REGDOMAIN_NO_5GHZ, { 'A', 'W'}},     //ARUBA
        { REGDOMAIN_WORLD, { 'A', 'X'}},     //ALAND ISLANDS
        { REGDOMAIN_NO_5GHZ, { 'A', 'Z'}},     //AZERBAIJAN
        { REGDOMAIN_WORLD, { 'B', 'A'}},     //BOSNIA AND HERZEGOVINA
        { REGDOMAIN_WORLD, { 'B', 'B'}},     //BARBADOS
        { REGDOMAIN_WORLD, { 'B', 'D'}},     //BANGLADESH
        { REGDOMAIN_ETSI,  {'B', 'E'}},      //BELGIUM
        { REGDOMAIN_WORLD, { 'B', 'F'}},     //BURKINA FASO
        { REGDOMAIN_HI_5GHZ, {'B', 'G'}},      //BULGARIA
        { REGDOMAIN_WORLD, { 'B', 'H'}},     //BAHRAIN
        { REGDOMAIN_WORLD, { 'B', 'I'}},     //BURUNDI
        { REGDOMAIN_WORLD, { 'B', 'J'}},     //BENIN
        { REGDOMAIN_ETSI, { 'B', 'M'}},     //BERMUDA
        { REGDOMAIN_WORLD, { 'B', 'N'}},     //BRUNEI DARUSSALAM
        { REGDOMAIN_WORLD,{ 'B', 'O'}},   //BOLIVIA
        { REGDOMAIN_WORLD, {'B', 'R'}},       //BRAZIL
        { REGDOMAIN_WORLD, { 'B', 'S'}},     //BAHAMAS
        { REGDOMAIN_WORLD, { 'B', 'T'}},     //BHUTAN
        { REGDOMAIN_WORLD, { 'B', 'V'}},     //BOUVET ISLAND
        { REGDOMAIN_WORLD, { 'B', 'W'}},     //BOTSWANA
        { REGDOMAIN_WORLD, { 'B', 'Y'}},     //BELARUS
        { REGDOMAIN_WORLD, { 'B', 'Z'}},     //BELIZE
        { REGDOMAIN_FCC, {'C', 'A'}},       //CANADA
        { REGDOMAIN_WORLD, { 'C', 'C'}},     //COCOS (KEELING) ISLANDS
        { REGDOMAIN_WORLD, { 'C', 'D'}},     //CONGO, THE DEMOCRATIC REPUBLIC OF THE
        { REGDOMAIN_WORLD, { 'C', 'F'}},     //CENTRAL AFRICAN REPUBLIC
        { REGDOMAIN_WORLD, { 'C', 'G'}},     //CONGO
        { REGDOMAIN_WORLD, {'C', 'H'}},      //SWITZERLAND
        { REGDOMAIN_WORLD, { 'C', 'I'}},     //COTE D'IVOIRE
        { REGDOMAIN_WORLD, { 'C', 'K'}},     //COOK ISLANDS
        { REGDOMAIN_WORLD, {'C', 'L'}},       //CHILE
        { REGDOMAIN_WORLD, { 'C', 'M'}},     //CAMEROON
        { REGDOMAIN_HI_5GHZ, {'C', 'N'}},   //CHINA
        { REGDOMAIN_WORLD, {'C', 'O'}},       //COLOMBIA
        { REGDOMAIN_WORLD, {'C', 'R'}},       //COSTA RICA
        { REGDOMAIN_WORLD, { 'C', 'U'}},     //CUBA
        { REGDOMAIN_WORLD, { 'C', 'V'}},     //CAPE VERDE
        { REGDOMAIN_WORLD, { 'C', 'X'}},     //CHRISTMAS ISLAND
        { REGDOMAIN_WORLD, {'C', 'Y'}},      //CYPRUS
        { REGDOMAIN_HI_5GHZ, {'C', 'Z'}},      //CZECH REPUBLIC
        { REGDOMAIN_ETSI, {'D', 'E'}},      //GERMANY
        { REGDOMAIN_WORLD, { 'D', 'J'}},     //DJIBOUTI
        { REGDOMAIN_ETSI, {'D', 'K'}},      //DENMARK
        { REGDOMAIN_WORLD, { 'D', 'M'}},     //DOMINICA
        { REGDOMAIN_NO_5GHZ,{ 'D', 'O'}},   //DOMINICAN REPUBLIC
        { REGDOMAIN_WORLD, { 'D', 'Z'}},     //ALGERIA
        { REGDOMAIN_WORLD,{ 'E', 'C'}},   //ECUADOR
        { REGDOMAIN_HI_5GHZ, {'E', 'E'}},      //ESTONIA
        { REGDOMAIN_WORLD, { 'E', 'G'}},     //EGYPT
        { REGDOMAIN_WORLD, { 'E', 'H'}},     //WESTERN SAHARA
        { REGDOMAIN_WORLD, { 'E', 'R'}},     //ERITREA
        { REGDOMAIN_ETSI, {'E', 'S'}},      //SPAIN
        { REGDOMAIN_WORLD, { 'E', 'T'}},     //ETHIOPIA
        { REGDOMAIN_WORLD, {'F', 'I'}},      //FINLAND
        { REGDOMAIN_WORLD, { 'F', 'J'}},     //FIJI
        { REGDOMAIN_WORLD, { 'F', 'K'}},     //FALKLAND ISLANDS (MALVINAS)
        { REGDOMAIN_WORLD, { 'F', 'M'}},     //MICRONESIA, FEDERATED STATES OF
        { REGDOMAIN_WORLD, { 'F', 'O'}},     //FAROE ISLANDS
        { REGDOMAIN_ETSI, {'F', 'R'}},      //FRANCE
        { REGDOMAIN_WORLD, { 'G', 'A'}},     //GABON
        { REGDOMAIN_ETSI, {'G', 'B'}},      //UNITED KINGDOM
        { REGDOMAIN_WORLD, { 'G', 'D'}},     //GRENADA
        { REGDOMAIN_HI_5GHZ, { 'G', 'E'}},     //GEORGIA
        { REGDOMAIN_WORLD, { 'G', 'F'}},     //FRENCH GUIANA
        { REGDOMAIN_ETSI, {'G', 'G'}},      //GUERNSEY
        { REGDOMAIN_WORLD, { 'G', 'H'}},     //GHANA
        { REGDOMAIN_WORLD, {'G', 'I'}},      //GIBRALTAR
        { REGDOMAIN_WORLD, { 'G', 'L'}},     //GREENLAND
        { REGDOMAIN_WORLD, { 'G', 'M'}},     //GAMBIA
        { REGDOMAIN_WORLD, { 'G', 'N'}},     //GUINEA
        { REGDOMAIN_WORLD, { 'G', 'P'}},     //GUADELOUPE
        { REGDOMAIN_WORLD, { 'G', 'Q'}},     //EQUATORIAL GUINEA
        { REGDOMAIN_ETSI, {'G', 'R'}},      //GREECE
        { REGDOMAIN_WORLD, { 'G', 'S'}},     //SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
        { REGDOMAIN_WORLD,{ 'G', 'T'}},   //GUATEMALA
        { REGDOMAIN_WORLD, { 'G', 'U'}},     //GUAM
        { REGDOMAIN_WORLD, { 'G', 'W'}},     //GUINEA-BISSAU
        { REGDOMAIN_WORLD, { 'G', 'Y'}},     //GUYANA
        { REGDOMAIN_WORLD, {'H', 'K'}},      //HONGKONG
        { REGDOMAIN_WORLD, { 'H', 'M'}},     //HEARD ISLAND AND MCDONALD ISLANDS
        { REGDOMAIN_WORLD,{'H', 'N'}},   //HONDURAS
        { REGDOMAIN_HI_5GHZ, {'H', 'R'}},      //CROATIA
        { REGDOMAIN_WORLD, { 'H', 'T'}},     //HAITI
        { REGDOMAIN_HI_5GHZ, {'H', 'U'}},      //HUNGARY
        { REGDOMAIN_APAC, { 'I', 'D'}},     //INDONESIA
        { REGDOMAIN_ETSI, {'I', 'E'}},     //IRELAND
        { REGDOMAIN_WORLD, {'I', 'L'}},        //ISREAL
        { REGDOMAIN_ETSI, {'I', 'M'}},      //ISLE OF MAN
        { REGDOMAIN_WORLD, {'I', 'N'}},      //INDIA
        { REGDOMAIN_ETSI, { 'I', 'O'}},     //BRITISH INDIAN OCEAN TERRITORY
        { REGDOMAIN_WORLD, { 'I', 'Q'}},     //IRAQ
        { REGDOMAIN_WORLD, { 'I', 'R'}},     //IRAN, ISLAMIC REPUBLIC OF
        { REGDOMAIN_WORLD, {'I', 'S'}},      //ICELAND
        { REGDOMAIN_ETSI, {'I', 'T'}},      //ITALY
        { REGDOMAIN_ETSI, {'J', 'E'}},      //JERSEY
        { REGDOMAIN_WORLD, { 'J', 'M'}},     //JAMAICA
        { REGDOMAIN_WORLD, { 'J', 'O'}},     //JORDAN
        { REGDOMAIN_JAPAN, {'J', 'P'}},     //JAPAN
        { REGDOMAIN_WORLD, { 'K', 'E'}},     //KENYA
        { REGDOMAIN_WORLD, { 'K', 'G'}},     //KYRGYZSTAN
        { REGDOMAIN_WORLD, { 'K', 'H'}},     //CAMBODIA
        { REGDOMAIN_WORLD, { 'K', 'I'}},     //KIRIBATI
        { REGDOMAIN_WORLD, { 'K', 'M'}},     //COMOROS
        { REGDOMAIN_WORLD, { 'K', 'N'}},     //SAINT KITTS AND NEVIS
        { REGDOMAIN_KOREA, { 'K', 'P'}},     //KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
        { REGDOMAIN_KOREA, {'K', 'R'}},     //KOREA, REPUBLIC OF
        { REGDOMAIN_WORLD, { 'K', 'W'}},     //KUWAIT
        { REGDOMAIN_WORLD, { 'K', 'Y'}},     //CAYMAN ISLANDS
        { REGDOMAIN_WORLD, { 'K', 'Z'}},     //KAZAKHSTAN
        { REGDOMAIN_WORLD, { 'L', 'A'}},     //LAO PEOPLE'S DEMOCRATIC REPUBLIC
        { REGDOMAIN_WORLD, { 'L', 'B'}},     //LEBANON
        { REGDOMAIN_WORLD, { 'L', 'C'}},     //SAINT LUCIA
        { REGDOMAIN_ETSI, {'L', 'I'}},      //LIECHTENSTEIN
        { REGDOMAIN_WORLD, { 'L', 'K'}},     //SRI LANKA
        { REGDOMAIN_WORLD, { 'L', 'R'}},     //LIBERIA
        { REGDOMAIN_WORLD, { 'L', 'S'}},     //LESOTHO
        { REGDOMAIN_HI_5GHZ, {'L', 'T'}},      //LITHUANIA
        { REGDOMAIN_ETSI, {'L', 'U'}},      //LUXEMBOURG
        { REGDOMAIN_HI_5GHZ, {'L', 'V'}},      //LATVIA
        { REGDOMAIN_WORLD, { 'L', 'Y'}},     //LIBYAN ARAB JAMAHIRIYA
        { REGDOMAIN_WORLD, { 'M', 'A'}},     //MOROCCO
        { REGDOMAIN_ETSI, {'M', 'C'}},      //MONACO
        { REGDOMAIN_WORLD, { 'M', 'D'}},     //MOLDOVA, REPUBLIC OF
        { REGDOMAIN_WORLD, { 'M', 'E'}},     //MONTENEGRO
        { REGDOMAIN_WORLD, { 'M', 'G'}},     //MADAGASCAR
        { REGDOMAIN_WORLD, { 'M', 'H'}},     //MARSHALL ISLANDS
        { REGDOMAIN_WORLD, { 'M', 'K'}},     //MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
        { REGDOMAIN_WORLD, { 'M', 'L'}},     //MALI
        { REGDOMAIN_WORLD, { 'M', 'M'}},     //MYANMAR
        { REGDOMAIN_HI_5GHZ, { 'M', 'N'}},     //MONGOLIA
        { REGDOMAIN_WORLD, { 'M', 'O'}},     //MACAO
        { REGDOMAIN_WORLD, { 'M', 'P'}},     //NORTHERN MARIANA ISLANDS
        { REGDOMAIN_WORLD, { 'M', 'Q'}},     //MARTINIQUE
        { REGDOMAIN_WORLD, { 'M', 'R'}},     //MAURITANIA
        { REGDOMAIN_WORLD, { 'M', 'S'}},     //MONTSERRAT
        { REGDOMAIN_WORLD, {'M', 'T'}},      //MALTA
        { REGDOMAIN_WORLD, { 'M', 'U'}},     //MAURITIUS
        { REGDOMAIN_WORLD, { 'M', 'V'}},     //MALDIVES
        { REGDOMAIN_WORLD, { 'M', 'W'}},     //MALAWI
        { REGDOMAIN_WORLD, {'M', 'X'}},       //MEXICO
        { REGDOMAIN_HI_5GHZ,{ 'M', 'Y'}},   //MALAYSIA
        { REGDOMAIN_WORLD, { 'M', 'Z'}},     //MOZAMBIQUE
        { REGDOMAIN_WORLD, { 'N', 'A'}},     //NAMIBIA
        { REGDOMAIN_WORLD, { 'N', 'C'}},     //NEW CALEDONIA
        { REGDOMAIN_WORLD, { 'N', 'E'}},     //NIGER
        { REGDOMAIN_WORLD, { 'N', 'F'}},     //NORFOLD ISLAND
        { REGDOMAIN_WORLD, { 'N', 'G'}},     //NIGERIA
        { REGDOMAIN_WORLD,{ 'N', 'I'}},       //NICARAGUA
        { REGDOMAIN_ETSI, {'N', 'L'}},      //NETHERLANDS
        { REGDOMAIN_WORLD, {'N', 'O'}},      //NORWAY
        { REGDOMAIN_WORLD, { 'N', 'P'}},     //NEPAL
        { REGDOMAIN_WORLD, { 'N', 'R'}},     //NAURU
        { REGDOMAIN_WORLD, { 'N', 'U'}},     //NIUE
        { REGDOMAIN_ETSI, {'N', 'Z'}},      //NEW ZEALAND
        { REGDOMAIN_WORLD, { 'O', 'M'}},     //OMAN
        { REGDOMAIN_WORLD, {'P', 'A'}},       //PANAMA
        { REGDOMAIN_WORLD,{ 'P', 'E'}},   //PERU
        { REGDOMAIN_WORLD, { 'P', 'F'}},     //FRENCH POLYNESIA
        { REGDOMAIN_WORLD, { 'P', 'G'}},     //PAPUA NEW GUINEA
        { REGDOMAIN_WORLD, {'P', 'H'}},      //PHILIPPINES
        { REGDOMAIN_WORLD, { 'P', 'K'}},     //PAKISTAN
        { REGDOMAIN_WORLD, {'P', 'L'}},      //POLAND
        { REGDOMAIN_WORLD, { 'P', 'M'}},     //SAINT PIERRE AND MIQUELON
        { REGDOMAIN_WORLD, { 'P', 'N'}},     //WORLDPITCAIRN
        { REGDOMAIN_FCC, {'P', 'R'}},       //PUERTO RICO
        { REGDOMAIN_WORLD, {'P', 'S'}},        //PALESTINIAN TERRITORY, OCCUPIED
        { REGDOMAIN_ETSI, {'P', 'T'}},      //PORTUGAL
        { REGDOMAIN_WORLD, { 'P', 'W'}},     //PALAU
        { REGDOMAIN_WORLD, { 'P', 'Y'}},     //PARAGUAY
        { REGDOMAIN_WORLD, { 'Q', 'A'}},     //QATAR
        { REGDOMAIN_WORLD, { 'R', 'E'}},     //REUNION
        { REGDOMAIN_HI_5GHZ, {'R', 'O'}},      //ROMANIA
        { REGDOMAIN_HI_5GHZ, {'R', 'S'}},      //SERBIA
        { REGDOMAIN_WORLD, {'R', 'U'}},   //RUSSIA
        { REGDOMAIN_WORLD, { 'R', 'W'}},     //RWANDA
        { REGDOMAIN_WORLD, {'S', 'A'}},      //SAUDI ARABIA
        { REGDOMAIN_WORLD, { 'S', 'B'}},     //SOLOMON ISLANDS
        { REGDOMAIN_ETSI, {'S', 'C'}},      //SEYCHELLES
        { REGDOMAIN_WORLD, { 'S', 'D'}},     //SUDAN
        { REGDOMAIN_ETSI, {'S', 'E'}},      //SWEDEN
        { REGDOMAIN_APAC, {'S', 'G'}},      //SINGAPORE
        { REGDOMAIN_WORLD, { 'S', 'H'}},     //SAINT HELENA
        { REGDOMAIN_HI_5GHZ, {'S', 'I'}},      //SLOVENNIA
        { REGDOMAIN_WORLD, { 'S', 'J'}},     //SVALBARD AND JAN MAYEN
        { REGDOMAIN_HI_5GHZ, {'S', 'K'}},      //SLOVAKIA
        { REGDOMAIN_WORLD, { 'S', 'L'}},     //SIERRA LEONE
        { REGDOMAIN_WORLD, { 'S', 'M'}},     //SAN MARINO
        { REGDOMAIN_WORLD, { 'S', 'N'}},     //SENEGAL
        { REGDOMAIN_WORLD, { 'S', 'O'}},     //SOMALIA
        { REGDOMAIN_WORLD, { 'S', 'R'}},     //SURINAME
        { REGDOMAIN_WORLD, { 'S', 'T'}},     //SAO TOME AND PRINCIPE
        { REGDOMAIN_WORLD, {'S', 'V'}},       //EL SALVADOR
        { REGDOMAIN_WORLD, { 'S', 'Y'}},     //SYRIAN ARAB REPUBLIC
        { REGDOMAIN_WORLD, { 'S', 'Z'}},     //SWAZILAND
        { REGDOMAIN_WORLD, { 'T', 'C'}},     //TURKS AND CAICOS ISLANDS
        { REGDOMAIN_WORLD, { 'T', 'D'}},     //CHAD
        { REGDOMAIN_WORLD, { 'T', 'F'}},     //FRENCH SOUTHERN TERRITORIES
        { REGDOMAIN_WORLD, { 'T', 'G'}},     //TOGO
        { REGDOMAIN_WORLD,{ 'T', 'H'}},   //THAILAND
        { REGDOMAIN_WORLD, { 'T', 'J'}},     //TAJIKISTAN
        { REGDOMAIN_WORLD, { 'T', 'K'}},     //TOKELAU
        { REGDOMAIN_WORLD, { 'T', 'L'}},     //TIMOR-LESTE
        { REGDOMAIN_WORLD, { 'T', 'M'}},     //TURKMENISTAN
        { REGDOMAIN_WORLD, { 'T', 'N'}},     //TUNISIA
        { REGDOMAIN_WORLD, { 'T', 'O'}},     //TONGA
        { REGDOMAIN_WORLD, {'T', 'R'}},      //TURKEY
        { REGDOMAIN_WORLD, { 'T', 'T'}},     //TRINIDAD AND TOBAGO
        { REGDOMAIN_WORLD, { 'T', 'V'}},     //TUVALU
        { REGDOMAIN_HI_5GHZ,{ 'T', 'W'}},   //TAIWAN, PROVINCE OF CHINA
        { REGDOMAIN_WORLD, { 'T', 'Z'}},     //TANZANIA, UNITED REPUBLIC OF
        { REGDOMAIN_HI_5GHZ,{ 'U', 'A'}},   //UKRAINE
        { REGDOMAIN_WORLD, { 'U', 'G'}},     //UGANDA
        { REGDOMAIN_FCC, {'U', 'M'}},       //UNITED STATES MINOR OUTLYING ISLANDS
        { REGDOMAIN_WORLD,{ 'U', 'Y'}},   //URUGUAY
        { REGDOMAIN_HI_5GHZ, { 'U', 'Z'}},     //UZBEKISTAN
        { REGDOMAIN_ETSI, {'V', 'A'}},      //HOLY SEE (VATICAN CITY STATE)
        { REGDOMAIN_WORLD, { 'V', 'C'}},     //SAINT VINCENT AND THE GRENADINES
        { REGDOMAIN_HI_5GHZ,{ 'V', 'E'}},   //VENEZUELA
        { REGDOMAIN_ETSI, {'V', 'G'}},       //VIRGIN ISLANDS, BRITISH
        { REGDOMAIN_FCC, {'V', 'I'}},       //VIRGIN ISLANDS, US
        { REGDOMAIN_WORLD, {'V', 'N'}},      //VIET NAM
        { REGDOMAIN_WORLD, { 'V', 'U'}},     //VANUATU
        { REGDOMAIN_WORLD, { 'W', 'F'}},     //WALLIS AND FUTUNA
        { REGDOMAIN_WORLD, { 'W', 'S'}},     //SOMOA
        { REGDOMAIN_WORLD, { 'Y', 'E'}},     //YEMEN
        { REGDOMAIN_WORLD, { 'Y', 'T'}},     //MAYOTTE
        { REGDOMAIN_WORLD, {'Z', 'A'}},      //SOUTH AFRICA
        { REGDOMAIN_WORLD, { 'Z', 'M'}},     //ZAMBIA
        { REGDOMAIN_WORLD, { 'Z', 'W'}},     //ZIMBABWE
        { REGDOMAIN_KOREA, {'K', '1'}},     //Korea alternate 1
        { REGDOMAIN_KOREA, {'K', '2'}},     //Korea alternate 2
        { REGDOMAIN_KOREA, {'K', '3'}},     //Korea alternate 3
        { REGDOMAIN_KOREA, {'K', '4'}},      //Korea alternate 4
        { REGDOMAIN_ETSI, {'E', 'U'}},       //Europe (SSGFI)
        { REGDOMAIN_JAPAN, {'J', '1'}},     //Japan alternate 1
        { REGDOMAIN_JAPAN, {'J', '2'}},     //Japan alternate 2
        { REGDOMAIN_JAPAN, {'J', '3'}},     //Japan alternate 3
        { REGDOMAIN_JAPAN, {'J', '4'}},     //Japan alternate 4
        { REGDOMAIN_JAPAN, {'J', '5'}}      //Japan alternate 5
    }
};
typedef struct nvEFSTable_s
{
   v_U32_t    nvValidityBitmap;
   sHalNv     halnv;
} nvEFSTable_t;
nvEFSTable_t *gnvEFSTable=NULL;
/*----------------------------------------------------------------------------
   Function Definitions and Documentation
 * -------------------------------------------------------------------------*/
VOS_STATUS wlan_write_to_efs (v_U8_t *pData, v_U16_t data_len);
/**------------------------------------------------------------------------
  \brief vos_nv_init() - initialize the NV module
  The \a vos_nv_init() initializes the NV module.  This read the binary
  file for country code and regulatory domain information.
  \return VOS_STATUS_SUCCESS - module is initialized successfully
          otherwise  - module is not initialized
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_init(void)
{
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS vos_nv_open(void)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_CONTEXT_t pVosContext= NULL;
    v_SIZE_t bufSize;
         /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    bufSize = sizeof(nvEFSTable_t);
    status = hdd_request_firmware(LIBRA_NV_FILE,((VosContextType*)(pVosContext))->pHDDContext,(v_VOID_t**)&gnvEFSTable,&bufSize);

    if ( (!VOS_IS_STATUS_SUCCESS( status )) || !gnvEFSTable)
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                         "%s : vos_nv_open failed!!! make sure the qcom_wlan_nv.bin is present in persist directory\n",__func__);

        return VOS_STATUS_E_FAILURE;
    }
    return VOS_STATUS_SUCCESS;
}

VOS_STATUS vos_nv_close(void)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_CONTEXT_t pVosContext= NULL;
         /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    status = hdd_release_firmware(LIBRA_NV_FILE,((VosContextType*)(pVosContext))->pHDDContext);
    if ( !VOS_IS_STATUS_SUCCESS( status ))
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                         "%s : vos_open failed\n",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    gnvEFSTable=NULL;
    return VOS_STATUS_SUCCESS;
}
/**------------------------------------------------------------------------
  \brief vos_nv_getRegDomainFromCountryCode() - get the regulatory domain of
  a country given its country code
  The \a vos_nv_getRegDomainFromCountryCode() returns the regulatory domain of
  a country given its country code.  This is done from reading a cached
  copy of the binary file.
  \param pRegDomain  - pointer to regulatory domain
  \param countryCode - country code
  \return VOS_STATUS_SUCCESS - regulatory domain is found for the given country
          VOS_STATUS_E_FAULT - invalid pointer error
          VOS_STATUS_E_EMPTY - country code table is empty
          VOS_STATUS_E_EXISTS - given country code does not exist in table
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_getRegDomainFromCountryCode( v_REGDOMAIN_t *pRegDomain,
      const v_COUNTRYCODE_t countryCode )
{
   int i;
   *pRegDomain = REGDOMAIN_COUNT;
   // sanity checks
   if (NULL == pRegDomain)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Invalid reg domain pointer\r\n") );
      return VOS_STATUS_E_FAULT;
   }
   if (NULL == countryCode)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Country code array is NULL\r\n") );
      return VOS_STATUS_E_FAULT;
   }
   if (0 == countryInfoTable.countryCount)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("Reg domain table is emtpy\r\n") );
      return VOS_STATUS_E_EMPTY;
   }
   // iterate the country info table until end of table or the country code
   // is found
   for (i = 0; i < countryInfoTable.countryCount &&
         REGDOMAIN_COUNT == *pRegDomain; i++)
   {
      if (memcmp(countryCode, countryInfoTable.countryInfo[i].countryCode,
               VOS_COUNTRY_CODE_LEN) == 0)
      {
         // country code is found
         *pRegDomain = countryInfoTable.countryInfo[i].regDomain;
      }
   }
   if (REGDOMAIN_COUNT != *pRegDomain)
   {
      return VOS_STATUS_SUCCESS;
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
            ("country code is not found\r\n"));
      return VOS_STATUS_E_EXISTS;
   }
}
/**------------------------------------------------------------------------
  \brief vos_nv_getSupportedCountryCode() - get the list of supported
  country codes
  The \a vos_nv_getSupportedCountryCode() encodes the list of supported
  country codes with paddings in the provided buffer
  \param pBuffer     - pointer to buffer where supported country codes
                       and paddings are encoded; this may be set to NULL
                       if user wishes to query the required buffer size to
                       get the country code list
  \param pBufferSize - this is the provided buffer size on input;
                       this is the required or consumed buffer size on output
  \return VOS_STATUS_SUCCESS - country codes are successfully encoded
          VOS_STATUS_E_NOMEM - country codes are not encoded because either
                               the buffer is NULL or buffer size is
                               sufficient
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_getSupportedCountryCode( v_BYTE_t *pBuffer, v_SIZE_t *pBufferSize,
      v_SIZE_t paddingSize )
{
   v_SIZE_t providedBufferSize = *pBufferSize;
   int i;
   // pBufferSize now points to the required buffer size
   *pBufferSize = countryInfoTable.countryCount * (VOS_COUNTRY_CODE_LEN + paddingSize );
   if ( NULL == pBuffer || providedBufferSize < *pBufferSize )
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            ("Insufficient memory for country code list\r\n"));
      return VOS_STATUS_E_NOMEM;
   }
   for (i = 0; i < countryInfoTable.countryCount; i++)
   {
      memcpy( pBuffer, countryInfoTable.countryInfo[i].countryCode, VOS_COUNTRY_CODE_LEN );
      pBuffer += (VOS_COUNTRY_CODE_LEN + paddingSize );
   }
   return VOS_STATUS_SUCCESS;
}
/**------------------------------------------------------------------------
  \brief vos_nv_readTxAntennaCount() - return number of TX antenna
  \param pTxAntennaCount   - antenna count
  \return status of the NV read operation
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_readTxAntennaCount( v_U8_t *pTxAntennaCount )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      *pTxAntennaCount = fieldImage.numOfTxChains;
   }
   return status;
}
/**------------------------------------------------------------------------
  \brief vos_nv_readRxAntennaCount() - return number of RX antenna
  \param pRxAntennaCount   - antenna count
  \return status of the NV read operation
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_readRxAntennaCount( v_U8_t *pRxAntennaCount )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      *pRxAntennaCount = fieldImage.numOfRxChains;
   }
   return status;
}

/**------------------------------------------------------------------------
  \brief vos_nv_readMacAddress() - return the MAC address
  \param pMacAddress - MAC address
  \return status of the NV read operation
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_readMacAddress( v_MAC_ADDRESS_t pMacAddress )
{
   sNvFields fieldImage;
   VOS_STATUS status;
   status = vos_nv_read( VNV_FIELD_IMAGE, &fieldImage, NULL,
         sizeof(fieldImage) );
   if (VOS_STATUS_SUCCESS == status)
   {
      memcpy( pMacAddress, fieldImage.macAddr, VOS_MAC_ADDRESS_LEN );
   }
   else
   {
      //This part of the code can be removed when NV is programmed
      const v_U8_t macAddr[VOS_MAC_ADDRESS_LEN] = VOS_HARD_CODED_MAC;
      memcpy( pMacAddress, macAddr, VOS_MAC_ADDRESS_LEN );
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
          " fail to get MAC address from NV, hardcoded to %02X-%02X-%02X-%02X-%02X%02X",
          macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
      status = VOS_STATUS_SUCCESS;
   }
   return status;
}
/**------------------------------------------------------------------------
  \brief vos_nv_setValidity() - set the validity of an NV item.
  The \a vos_nv_setValidity() validates and invalidates an NV item.  The
  validity information is stored in NV memory.
  One would get the VOS_STATUS_E_EXISTS error when reading an invalid item.
  An item becomes valid when one has written to it successfully.
  \param type        - NV item type
  \param itemIsValid - boolean value indicating the item's validity
  \return VOS_STATUS_SUCCESS - validity is set successfully
          VOS_STATUS_E_INVAL - one of the parameters is invalid
          VOS_STATUS_E_FAILURE - unknown error
  \sa
  -------------------------------------------------------------------------*/
#ifndef WLAN_FTM_STUB

VOS_STATUS vos_nv_setValidity( VNV_TYPE type, v_BOOL_t itemIsValid )
{
   v_U32_t lastNvValidityBitmap;
   v_U32_t newNvValidityBitmap;
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   // check if the current NV type is valid
   if (VNV_TYPE_COUNT < type)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("vos_nv_setValidity: invalid type=%d\r\n"), type );
      return VOS_STATUS_E_INVAL;
   }
   // read the validity bitmap
   lastNvValidityBitmap = gnvEFSTable->nvValidityBitmap;
   // modify the validity bitmap
   if (itemIsValid)
   {
       newNvValidityBitmap = lastNvValidityBitmap | (1 << type);
              // commit to NV store if bitmap has been modified
       if (newNvValidityBitmap != lastNvValidityBitmap)
       {
           gnvEFSTable->nvValidityBitmap = newNvValidityBitmap;
       }
   }
   else
   {
       newNvValidityBitmap = lastNvValidityBitmap & (~(1 << type));
       if (newNvValidityBitmap != lastNvValidityBitmap)
       {
           gnvEFSTable->nvValidityBitmap = newNvValidityBitmap;
           status = wlan_write_to_efs((v_U8_t*)gnvEFSTable,sizeof(nvEFSTable_t));
           if (! VOS_IS_STATUS_SUCCESS(status)) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_write_to_efs failed!!!\r\n"));
               status = VOS_STATUS_E_FAULT;
           }
       }
   }

   return status;
}
#endif
/**------------------------------------------------------------------------
  \brief vos_nv_getValidity() - get the validity of an NV item.
  The \a vos_nv_getValidity() indicates if an NV item is valid.  The
  validity information is stored in NV memory.
  One would get the VOS_STATUS_E_EXISTS error when reading an invalid item.
  An item becomes valid when one has written to it successfully.
  \param type        - NV item type
  \param pItemIsValid- pointer to the boolean value indicating the item's
                       validity
  \return VOS_STATUS_SUCCESS - validity is determined successfully
          VOS_STATUS_E_INVAL - one of the parameters is invalid
          VOS_STATUS_E_FAILURE - unknown error
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_getValidity( VNV_TYPE type, v_BOOL_t *pItemIsValid )
{
   v_U32_t nvValidityBitmap = gnvEFSTable->nvValidityBitmap;
   // check if the current NV type is valid
   if (NUM_NV_TABLE_IDS < type)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            ("vos_nv_getValidity: invalid type=%d\r\n"), type );
      return VOS_STATUS_E_INVAL;
   }
   *pItemIsValid = (v_BOOL_t)((nvValidityBitmap >> type) & 1);
   return VOS_STATUS_SUCCESS;
}
/**------------------------------------------------------------------------
  \brief vos_nv_read() - read a NV item to an output buffer
  The \a vos_nv_read() reads a NV item to an output buffer.  If the item is
  an array, this function would read the entire array. One would get a
  VOS_STATUS_E_EXISTS error when reading an invalid item.
  For error conditions of VOS_STATUS_E_EXISTS and VOS_STATUS_E_FAILURE,
  if a default buffer is provided (with a non-NULL value),
  the default buffer content is copied to the output buffer.
  \param type  - NV item type
  \param outputBuffer   - output buffer
  \param defaultBuffer  - default buffer
  \param bufferSize  - output buffer size
  \return VOS_STATUS_SUCCESS - NV item is read successfully
          VOS_STATUS_E_INVAL - one of the parameters is invalid
          VOS_STATUS_E_FAULT - defaultBuffer point is NULL
          VOS_STATUS_E_EXISTS - NV type is unsupported
          VOS_STATUS_E_FAILURE - unknown error
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_read( VNV_TYPE type, v_VOID_t *outputVoidBuffer,
      v_VOID_t *defaultBuffer, v_SIZE_t bufferSize )
{
    VOS_STATUS status             = VOS_STATUS_SUCCESS;
    v_SIZE_t itemSize;
    v_BOOL_t itemIsValid = VOS_TRUE;

    // sanity check
    if (VNV_TYPE_COUNT < type)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("vos_nv_setValidity: invalid type=%d\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    if (NULL == outputVoidBuffer)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("Buffer provided is NULL\r\n") );
       return VOS_STATUS_E_FAULT;
    }
    if (0 == bufferSize)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("NV type=%d is invalid\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    // check if the NV item has valid data
    status = vos_nv_getValidity( type, &itemIsValid );
   if (!itemIsValid)
   {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN,
            "NV type=%d does not have valid data\r\n", type );
       return VOS_STATUS_E_EMPTY;
   }
   switch(type)
   {
       case VNV_FIELD_IMAGE:
           itemSize = sizeof(gnvEFSTable->halnv.fields);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.fields,bufferSize);
           }
           break;
       case VNV_RATE_TO_POWER_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.pwrOptimum[0],bufferSize);
           }
           break;
       case VNV_REGULARTORY_DOMAIN_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.regDomains);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.regDomains[0],bufferSize);
           }
           break;
       case VNV_DEFAULT_LOCATION:
           itemSize = sizeof(gnvEFSTable->halnv.tables.defaultCountryTable);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.defaultCountryTable,bufferSize);
           }
           break;
       case VNV_TPC_POWER_TABLE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.plutCharacterized);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.plutCharacterized[0],bufferSize);
           }
           break;
       case VNV_TPC_PDADC_OFFSETS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.plutPdadcOffset);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.plutPdadcOffset[0],bufferSize);
           }
           break;
       case VNV_RSSI_CHANNEL_OFFSETS:

           itemSize = sizeof(gnvEFSTable->halnv.tables.rssiChanOffsets);

           if(bufferSize != itemSize) {

               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.rssiChanOffsets[0],bufferSize);
           }
           break;
       case VNV_RF_CAL_VALUES:

           itemSize = sizeof(gnvEFSTable->halnv.tables.rFCalValues);

           if(bufferSize != itemSize) {

               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.rFCalValues,bufferSize);
           }
           break;
       case VNV_ANTENNA_PATH_LOSS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.antennaPathLoss);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.antennaPathLoss[0],bufferSize);
           }
           break;
       case VNV_PACKET_TYPE_POWER_LIMITS:
           itemSize = sizeof(gnvEFSTable->halnv.tables.pktTypePwrLimits);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.pktTypePwrLimits[0][0],bufferSize);
           }
           break;
       case VNV_OFDM_CMD_PWR_OFFSET:
           itemSize = sizeof(gnvEFSTable->halnv.tables.ofdmCmdPwrOffset);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.ofdmCmdPwrOffset,bufferSize);
           }
           break;
       case VNV_TX_BB_FILTER_MODE:
           itemSize = sizeof(gnvEFSTable->halnv.tables.txbbFilterMode);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.txbbFilterMode,bufferSize);
           }
           break;
       case VNV_FREQUENCY_FOR_1_3V_SUPPLY:
           itemSize = sizeof(gnvEFSTable->halnv.tables.freqFor1p3VSupply);
           if(bufferSize != itemSize) {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                 itemSize);
               status = VOS_STATUS_E_INVAL;
           }
           else {
               memcpy(outputVoidBuffer,&gnvEFSTable->halnv.tables.freqFor1p3VSupply,bufferSize);
           }
           break;

       default:
         break;
   }
   return status;
}
#ifndef WLAN_FTM_STUB

/**------------------------------------------------------------------------
  \brief vos_nv_write() - write to a NV item from an input buffer
  The \a vos_nv_write() writes to a NV item from an input buffer. This would
  validate the NV item if the write operation is successful.
  \param type - NV item type
  \param inputBuffer - input buffer
  \param inputBufferSize - input buffer size
  \return VOS_STATUS_SUCCESS - NV item is read successfully
          VOS_STATUS_E_INVAL - one of the parameters is invalid
          VOS_STATUS_E_FAULT - outputBuffer pointer is NULL
          VOS_STATUS_E_EXISTS - NV type is unsupported
          VOS_STATUS_E_FAILURE   - unknown error
  \sa
  -------------------------------------------------------------------------*/
VOS_STATUS vos_nv_write( VNV_TYPE type, v_VOID_t *inputVoidBuffer,
      v_SIZE_t bufferSize )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    v_SIZE_t itemSize;
        // sanity check
    if (NUM_NV_TABLE_IDS < type)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("vos_nv_setValidity: invalid type=%d\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    if (NULL == inputVoidBuffer)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("Buffer provided is NULL\r\n") );
       return VOS_STATUS_E_FAULT;
    }
    if (0 == bufferSize)
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             ("NV type=%d is invalid\r\n"), type );
       return VOS_STATUS_E_INVAL;
    }
    switch(type)
    {
        case VNV_FIELD_IMAGE:
            itemSize = sizeof(gnvEFSTable->halnv.fields);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.fields,inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_RATE_TO_POWER_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.pwrOptimum);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.pwrOptimum[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_REGULARTORY_DOMAIN_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.regDomains);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.regDomains[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_DEFAULT_LOCATION:
            itemSize = sizeof(gnvEFSTable->halnv.tables.defaultCountryTable);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.defaultCountryTable,inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_TPC_POWER_TABLE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.plutCharacterized);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.plutCharacterized[0],inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_TPC_PDADC_OFFSETS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.plutPdadcOffset);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.plutPdadcOffset[0],inputVoidBuffer,bufferSize);
            }
            break;
         case VNV_RSSI_CHANNEL_OFFSETS:

            itemSize = sizeof(gnvEFSTable->halnv.tables.rssiChanOffsets);

            if(bufferSize != itemSize) {

                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.rssiChanOffsets[0],inputVoidBuffer,bufferSize);
            }
            break;
         case VNV_RF_CAL_VALUES:

            itemSize = sizeof(gnvEFSTable->halnv.tables.rFCalValues);

            if(bufferSize != itemSize) {

                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.rFCalValues,inputVoidBuffer,bufferSize);
            }
            break;
        case VNV_ANTENNA_PATH_LOSS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.antennaPathLoss);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.antennaPathLoss[0],inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_PACKET_TYPE_POWER_LIMITS:
            itemSize = sizeof(gnvEFSTable->halnv.tables.pktTypePwrLimits);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.pktTypePwrLimits[0][0],inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_OFDM_CMD_PWR_OFFSET:
            itemSize = sizeof(gnvEFSTable->halnv.tables.ofdmCmdPwrOffset);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.ofdmCmdPwrOffset,inputVoidBuffer,bufferSize);
            }
            break;

        case VNV_TX_BB_FILTER_MODE:
            itemSize = sizeof(gnvEFSTable->halnv.tables.txbbFilterMode);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.txbbFilterMode,inputVoidBuffer,bufferSize);
            }
            break;
            
        case VNV_FREQUENCY_FOR_1_3V_SUPPLY:
            itemSize = sizeof(gnvEFSTable->halnv.tables.freqFor1p3VSupply);
            if(bufferSize != itemSize) {
                VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 ("type = %d buffer size=%d is less than data size=%d\r\n"),type, bufferSize,
                  itemSize);
                status = VOS_STATUS_E_INVAL;
            }
            else {
                memcpy(&gnvEFSTable->halnv.tables.freqFor1p3VSupply,inputVoidBuffer,bufferSize);
            }
            break;

        default:
          break;
    }
   if (VOS_STATUS_SUCCESS == status)
   {
      // set NV item to have valid data
      status = vos_nv_setValidity( type, VOS_TRUE );
      if (! VOS_IS_STATUS_SUCCESS(status)) {
          VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_setValidity failed!!!\r\n"));
          status = VOS_STATUS_E_FAULT;
      }
      status = wlan_write_to_efs((v_U8_t*)gnvEFSTable,sizeof(nvEFSTable_t));

      if (! VOS_IS_STATUS_SUCCESS(status)) {
          VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, ("vos_nv_write_to_efs failed!!!\r\n"));
          status = VOS_STATUS_E_FAULT;
      }
   }
   return status;
}
#endif
