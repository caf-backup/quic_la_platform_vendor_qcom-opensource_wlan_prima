/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halEeprom.h

    \brief halEeprom services

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALEEPROM_H
#define HALEEPROM_H

#include "halEepromTables.h"

typedef enum
{
    EEPROM_CONFIG_NONE,
    EEPROM_CONFIG_PCI,
    EEPROM_CONFIG_PCIE,
    EEPROM_CONFIG_USB
}eHostIFCfgType;


typedef enum
{
    // ASIC PCI config
    EEPROM_PCI_EEPROM_KEY,                  // 0
    EEPROM_PCI_REVISION_ID,                 // 1
    EEPROM_PCI_DEVICE_ID,                   // 2
    EEPROM_PCI_PMI_PARAMS,                  // 3
    EEPROM_PCI_CLASS_CODE,                  // 4
    EEPROM_PCI_SUBSYSTEM_VENDOR_ID,         // 5
    EEPROM_PCI_SUBSYSTEM_ID,                // 6
    EEPROM_PCI_CARDBUS_PTR,                 // 7
    EEPROM_PCI_VENDOR_ID,                   // 8
    EEPROM_PCI_MIN_GRANT,                   // 9
    EEPROM_PCI_MAX_LATENCY,                 // 10
    EEPROM_PCI_BASE_ADDR_CTL,               // 11
    EEPROM_PCI_RESERVED,                    // 12
    EEPROM_PCI_ADDITIONAL_PMC,              // 13
    EEPROM_PCI_VPD_BASE_ADDR,               // 14

    //all additions to the EEPROM fields should be numbered consecutively from 200
    //this is intended to keep us from needing to update external applications for existing fields
    EEPROM_PCI_CIS_DATA                     = 200,

    // ASIC USB CONFIG
    EEPROM_USB_EEPROM_KEY                   = 15,
    EEPROM_USB_BCD_USB,                     // 16
    EEPROM_USB_CLASS_CODE,                  // 17
    EEPROM_USB_SUB_CLASS_CODE,              // 18
    EEPROM_USB_PROTOCOL_CODE,               // 19
    EEPROM_USB_VENDOR_ID,                   // 20
    EEPROM_USB_PRODUCT_ID,                  // 21
    EEPROM_USB_DEV_RELEASE,                 // 22
    EEPROM_USB_MFG_STR_DESC_INDX,           // 23
    EEPROM_USB_PROD_STR_DESC_INDX,          // 24
    EEPROM_USB_SER_STR_DESC_INDX,           // 25
    EEPROM_USB_HS_CFG,                      // 26
    EEPROM_USB_HS_CFG_STR_DESC_INDX,        // 27
    EEPROM_USB_HS_DEVICE_ATTRIBUTES,        // 28
    EEPROM_USB_HS_MAXPWR,                   // 29
    EEPROM_USB_HS_INTF_CLASS_CODE,          // 30
    EEPROM_USB_HS_INTF_SUB_CLASS_CODE,      // 31
    EEPROM_USB_HS_INTF_PROTOCOL_CODE,       // 32
    EEPROM_USB_HS_INTF_STR_DESC_INDX,       // 33
    EEPROM_USB_HS_ENDPOINT1_MAX_PKT_SIZE,   // 34
    EEPROM_USB_HS_ENDPOINT1_B_INTERVAL,     // 35
    EEPROM_USB_HS_ENDPOINT2_MAX_PKT_SIZE,   // 36
    EEPROM_USB_HS_ENDPOINT2_B_INTERVAL,     // 37
    EEPROM_USB_HS_ENDPOINT3_MAX_PKT_SIZE,   // 38
    EEPROM_USB_HS_ENDPOINT3_B_INTERVAL,     // 39
    EEPROM_USB_HS_ENDPOINT4_MAX_PKT_SIZE,   // 40
    EEPROM_USB_HS_ENDPOINT4_B_INTERVAL,     // 41
    EEPROM_USB_HS_ENDPOINT5_MAX_PKT_SIZE,   // 42
    EEPROM_USB_HS_ENDPOINT5_B_INTERVAL,     // 43
    EEPROM_USB_HS_ENDPOINT6_MAX_PKT_SIZE,   // 44
    EEPROM_USB_HS_ENDPOINT6_B_INTERVAL,     // 45
    EEPROM_USB_HS_ENDPOINT7_MAX_PKT_SIZE,   // 46
    EEPROM_USB_HS_ENDPOINT7_B_INTERVAL,     // 47
    EEPROM_USB_HS_ENDPOINT8_MAX_PKT_SIZE,   // 48
    EEPROM_USB_HS_ENDPOINT8_B_INTERVAL,     // 49
    EEPROM_USB_HS_ENDPOINT9_MAX_PKT_SIZE,   // 50
    EEPROM_USB_HS_ENDPOINT9_B_INTERVAL,     // 51
    EEPROM_USB_HS_ENDPOINT10_MAX_PKT_SIZE,  // 52
    EEPROM_USB_HS_ENDPOINT10_B_INTERVAL,    // 53
    EEPROM_USB_HS_ENDPOINT11_MAX_PKT_SIZE,  // 54
    EEPROM_USB_HS_ENDPOINT11_B_INTERVAL,    // 55
    EEPROM_USB_FS_CFG,                      // 56
    EEPROM_USB_FS_CFG_STR_DESC_INDX,        // 57
    EEPROM_USB_FS_DEVICE_ATTRIBUTES,        // 58
    EEPROM_USB_FS_MAXPWR,                   // 59
    EEPROM_USB_FS_INTF_CLASS_CODE,          // 60
    EEPROM_USB_FS_INTF_SUB_CLASS_CODE,      // 61
    EEPROM_USB_FS_INTF_PROTOCOL_CODE,       // 62
    EEPROM_USB_FS_INTF_STR_DESC_INDX,       // 63
    EEPROM_USB_FS_ENDPOINT1_MAX_PKT_SIZE,   // 64
    EEPROM_USB_FS_ENDPOINT1_B_INTERVAL,     // 65
    EEPROM_USB_FS_ENDPOINT2_MAX_PKT_SIZE,   // 66
    EEPROM_USB_FS_ENDPOINT2_B_INTERVAL,     // 67
    EEPROM_USB_FS_ENDPOINT3_MAX_PKT_SIZE,   // 68
    EEPROM_USB_FS_ENDPOINT3_B_INTERVAL,     // 69
    EEPROM_USB_FS_ENDPOINT4_MAX_PKT_SIZE,   // 70
    EEPROM_USB_FS_ENDPOINT4_B_INTERVAL,     // 71
    EEPROM_USB_FS_ENDPOINT5_MAX_PKT_SIZE,   // 72
    EEPROM_USB_FS_ENDPOINT5_B_INTERVAL,     // 73
    EEPROM_USB_FS_ENDPOINT6_MAX_PKT_SIZE,   // 74
    EEPROM_USB_FS_ENDPOINT6_B_INTERVAL,     // 75
    EEPROM_USB_FS_ENDPOINT7_MAX_PKT_SIZE,   // 76
    EEPROM_USB_FS_ENDPOINT7_B_INTERVAL,     // 77
    EEPROM_USB_FS_ENDPOINT8_MAX_PKT_SIZE,   // 78
    EEPROM_USB_FS_ENDPOINT8_B_INTERVAL,     // 79
    EEPROM_USB_FS_ENDPOINT9_MAX_PKT_SIZE,   // 80
    EEPROM_USB_FS_ENDPOINT9_B_INTERVAL,     // 81
    EEPROM_USB_FS_ENDPOINT10_MAX_PKT_SIZE,  // 82
    EEPROM_USB_FS_ENDPOINT10_B_INTERVAL,    // 83
    EEPROM_USB_FS_ENDPOINT11_MAX_PKT_SIZE,  // 84
    EEPROM_USB_FS_ENDPOINT11_B_INTERVAL,    // 85
    EEPROM_USB_STR_DESC0_LANG_ID_CODE0,     // 86
    EEPROM_USB_RESERVED,                    // 87
    EEPROM_USB_MFG_STR,                     // 88
    EEPROM_USB_PROD_STR,                    // 89
    EEPROM_USB_SERIAL_STR,                  // 90
    EEPROM_USB_HS_CFG_STR,                  // 91
    EEPROM_USB_FS_CFG_STR,                  // 92
    EEPROM_USB_INTERFACE_STR,               // 93

    //PCI Express Config Fields
    EEPROM_PCIE_EEPROM_KEY,                 // 94
    EEPROM_PCIE_BAR2_ADDR_64BIT,            // 95
    EEPROM_PCIE_RESERVED,                   // 96
    EEPROM_PCIE_VENDOR_ID,                  // 97
    EEPROM_PCIE_DEVICE_ID,                  // 98
    EEPROM_PCIE_PCIEXPRESS_REVISION_ID,     // 99
    EEPROM_PCIE_CLASS_CODE,                 // 100
    EEPROM_PCIE_SUBSYSTEM_VENDOR_ID,        // 101
    EEPROM_PCIE_SUBSYSTEM_ID,               // 102
    EEPROM_PCIE_CARDBUS_POINTER,            // 103
    EEPROM_PCIE_POWER_MANAGEMENT_CONTROL,   // 104
    EEPROM_PCIE_DEVICE_CAPABILITY_REGISTER, // 105
    EEPROM_PCIE_LINK_CAPABILITY_REGISTER,   // 106
    EEPROM_PCIE_RESERVED2,                  // 107

    //Common Eeprom Fields
    EEPROM_COMMON_PRODUCT_ID,               // 108
    EEPROM_COMMON_EEP_IMAGE_VERSION,        // 109
    EEPROM_COMMON_EEP_SIZE,                 // 110
    EEPROM_COMMON_TABLE_DIRECTORY_OFFSET,   // 111
    EEPROM_COMMON_RF_CHIP_VERSION,          // 112
    EEPROM_COMMON_MAC_CHIP_VERSION,         // 113
    EEPROM_COMMON_CARD_TYPE,                // 114
    EEPROM_COMMON_NUM_OF_TX_CHAINS,         // 115
    EEPROM_COMMON_NUM_OF_RX_CHAINS,         // 116
    EEPROM_COMMON_PRODUCT_BANDS,            // 117
    EEPROM_COMMON_PDADC_GAIN_2_4_GHZ,       // 118
    EEPROM_COMMON_PDADC_GAIN_5_GHZ,         // 119
    EEPROM_COMMON_RATE_EN_HW_LIMITS,        // 120
    EEPROM_COMMON_CAPABILITY_HW_LIMITS,     // 121
    EEPROM_COMMON_SDRAM_INFO,               // 122
    EEPROM_COMMON_MAC_ADDR,                 // 123
    EEPROM_COMMON_CLOCK_SPEED,              // 124
    EEPROM_COMMON_GOLDEN_UNIT,              // 125
    EEPROM_COMMON_COUNTRY_CODE,             // 126
    EEPROM_COMMON_DEFAULT_REG_DOMAIN,       // 127
    EEPROM_COMMON_MFG_BOARD_NUM,            // 128
    EEPROM_COMMON_MFG_BOARD_ASSEMBLY_REV,   // 129
    EEPROM_COMMON_MFG_TEST_STATUS,          // 130
    EEPROM_COMMON_MFG_USAGE_RESTRICT,       // 131
    EEPROM_COMMON_RADIO_POWER_OFF_SWITCH,   // 132
    EEPROM_COMMON_CHAN_BOND_ALLOWED,        // 133
    EEPROM_COMMON_LNA_SWITCHABLE,           // 134
    EEPROM_COMMON_NUM_STATIONS,             // 135
    EEPROM_COMMON_UNUSED,                   // 136
    EEPROM_COMMON_RATE_EN_SW_LIMITS,        // 137
    EEPROM_COMMON_CAPABILITY_SW_LIMITS,     // 138
    EEPROM_COMMON_REG_ACCESS,               // 139
    EEPROM_COMMON_INTERNAL_SRAM_SIZE,       // 140
    EEPROM_COMMON_RESERVED,                 // 141
    EEPROM_COMMON_CKSUM_PRECEDING,          // 142
    EEPROM_COMMON_CKSUM_TABLES,             // 143

    NUM_EEPROM_FIELDS

}eEepromField;

typedef enum
{
    SDRAM_NOT_PRESENT = 0,
    SDRAM_FPGA        = 1,
    SDRAM_AEVB        = 2,
    
    NUM_SDRAM_OPTIONS
}eEepromSdram;                  //EEPROM_COMMON_SDRAM_INFO

#ifndef ANI_MANF_DIAG
typedef enum
{
    PRODUCT_BAND_11_B_G     = 0,
    PRODUCT_BAND_11_A_B_G   = 1,
    PRODUCT_BAND_11_A       = 2,
    
    NUM_PRODUCT_BANDS
}eEepromProductBands;           //EEPROM_COMMON_PRODUCT_BANDS 
#endif

typedef union
{
    /* Some of the following fields are commented out to avoid duplicate names
        It doesn't really matter since they are only here for completeness and
        the union names aren't really used anyway.
    */


    //USB fields
    tANI_U8  eepromKey;
    tANI_U8  bcdUSB[2];
    tANI_U8  classCode;
    tANI_U8  subClassCode;
    tANI_U8  protocolCode;
    tANI_U16 vendorId;
    //tANI_U16 productId;
    tANI_U16 devRelease;
    tANI_U8 mfgStrDescIndx;
    tANI_U8 prodStrDescIndx;
    tANI_U8 serStrDescIndx;
    tANI_U8 hsCfg;
    tANI_U8 hsCfgStrDescIndx;
    tANI_U8 hsDeviceAttributes;
    tANI_U8 hsMaxpwr;
    tANI_U8 hsIntfClassCode;
    tANI_U8 hsIntfSubClassCode;
    tANI_U8 hsIntfProtocolCode;
    tANI_U8 hsIntfStrDescIndx;
    tANI_U8 hsEndpoint1MaxPktSize;
    tANI_U8 hsEndpoint1bInterval;
    tANI_U8 hsEndpoint2MaxPktSize;
    tANI_U8 hsEndpoint2bInterval;
    tANI_U8 hsEndpoint3MaxPktSize;
    tANI_U8 hsEndpoint3bInterval;
    tANI_U8 hsEndpoint4MaxPktSize;
    tANI_U8 hsEndpoint4bInterval;
    tANI_U8 hsEndpoint5MaxPktSize;
    tANI_U8 hsEndpoint5bInterval;
    tANI_U8 hsEndpoint6MaxPktSize;
    tANI_U8 hsEndpoint6bInterval;
    tANI_U8 hsEndpoint7MaxPktSize;
    tANI_U8 hsEndpoint7bInterval;
    tANI_U8 hsEndpoint8MaxPktSize;
    tANI_U8 hsEndpoint8bInterval;
    tANI_U8 hsEndpoint9MaxPktSize;
    tANI_U8 hsEndpoint9bInterval;
    tANI_U8 hsEndpoint10MaxPktSize;
    tANI_U8 hsEndpoint10bInterval;
    tANI_U8 hsEndpoint11MaxPktSize;
    tANI_U8 hsEndpoint11bInterval;
    tANI_U8 fsCfg;
    tANI_U8 fsCfgStrDescIndx;
    tANI_U8 fsDeviceAttributes;
    tANI_U8 fsMaxpwr;
    tANI_U8 fsIntfClassCode;
    tANI_U8 fsIntfSubClassCode;
    tANI_U8 fsIntfProtocolCode;
    tANI_U8 fsIntfStrDescIndx;
    tANI_U8 fsEndpoint1MaxPktSize;
    tANI_U8 fsEndpoint1bInterval;
    tANI_U8 fsEndpoint2MaxPktSize;
    tANI_U8 fsEndpoint2bInterval;
    tANI_U8 fsEndpoint3MaxPktSize;
    tANI_U8 fsEndpoint3bInterval;
    tANI_U8 fsEndpoint4MaxPktSize;
    tANI_U8 fsEndpoint4bInterval;
    tANI_U8 fsEndpoint5MaxPktSize;
    tANI_U8 fsEndpoint5bInterval;
    tANI_U8 fsEndpoint6MaxPktSize;
    tANI_U8 fsEndpoint6bInterval;
    tANI_U8 fsEndpoint7MaxPktSize;
    tANI_U8 fsEndpoint7bInterval;
    tANI_U8 fsEndpoint8MaxPktSize;
    tANI_U8 fsEndpoint8bInterval;
    tANI_U8 fsEndpoint9MaxPktSize;
    tANI_U8 fsEndpoint9bInterval;
    tANI_U8 fsEndpoint10MaxPktSize;
    tANI_U8 fsEndpoint10bInterval;
    tANI_U8 fsEndpoint11MaxPktSize;
    tANI_U8 fsEndpoint11bInterval;
    tANI_U8 strDesc0LangIdCode0[2];
    tANI_U8 mfgStr[32];
    tANI_U8 prodStr[32];
    tANI_U8 serialStr[32];
    tANI_U8 hsCfgStr[32];
    tANI_U8 fsCfgStr[32];
    tANI_U8 interfaceStr[32];

    //PCI fields
    //tANI_U8  eepromKey;
    tANI_U8  revisionId;
    tANI_U16 deviceId;
    tANI_U8  pmiParams;
    //tANI_U8  classCode[3];
    tANI_U16 subsystemVendorId;
    tANI_U16 subsystemId;
    tANI_U8  cardbusPtr[4];
    //tANI_U16 vendorId;
    tANI_U8  minGrant;
    tANI_U8  maxLatency;
    tANI_U32 baseAddrCtl;
    tANI_U8  additionalPmc;
    tANI_U16 vpdBaseAddr;
    tANI_U8  cisData[210];                    // CIS data only written for Cardbus card types
    
    //PCIe fields
    //tANI_U8 eepromKey;
    tANI_U8 bar2Addr64bit;
    //tANI_U16 vendorId;
    //tANI_U16 deviceId;
    tANI_U8 pciExpressRevisionId;
    //tANI_U8 classCode[3];
    //tANI_U16 subsystemVendorId;
    //tANI_U16 subsystemId;
    tANI_U32 cardbusPointer;
    tANI_U16 powerManagementControl;
    tANI_U16 deviceCapabilityRegister;
    tANI_U32 linkCapabilityRegister;


    //common EEPROM fields
    tANI_U16  productId;
    tANI_U8   eepImageVersion;
    tANI_U8   eepSize;

    tANI_U16  tableDirectoryOffset; //byte offset where the table directory begins
    tANI_U8   rfChipVersion;
    tANI_U8   macChipVersion;

    tANI_U8   cardType;
    tANI_U8   numOfTxChains;
    tANI_U8   numOfRxChains;
    tANI_U8   productBands;

    tANI_U8   padcGain_24ghz;
    tANI_U8   padcGain_5ghz;

    tANI_U8   rateEnableHwLimits[32];

    tANI_U8   capabilityHwLimits[8];

    tANI_U8   sdramInfo;

    tANI_U8   macAddr[6];
    tANI_U8   clockSpeed;               //120 vs. 240 MHz
    tANI_U8   goldenUnit;

    tANI_U8   countryCode[3];        // Country Code
    tANI_U8   defaultRegDomain;      // must match the country code

    tANI_U8   mfgBoardNum[40];

    tANI_U8   mfgBoardAssemblyRev;
    tANI_U8   mfgTestStatus;
    tANI_U8   mfgUsageRestrict;
    tANI_U8   radioPowerOffSwitch;

    tANI_BOOLEAN chanBondAllowed;
    tANI_BOOLEAN lnaSwitchable;
    tANI_U8   numStations;
    tANI_U8   unused;

    tANI_U8   rateEnableSwLimits[32];

    tANI_U8   capabilitySwLimits[8];

    tANI_U8   phyRegAccess;

    tANI_U32  cksumPreceding;

    tANI_U32  cksumTables;

}uEepromFields;

// Card type definition
#define CARD_TYPE_MINIPCI   0
#define CARD_TYPE_CARDBUS   1
#define CARD_TYPE_APX       2
#define CARD_TYPE_USB       3
#define CARD_TYPE_MINIPCIX  4
#define CARD_TYPE_PCIX      5
#define CARD_NOT_SPECIFIED  0xFF


#define MAX_EEPROM_TABLE    30

#define EEPROM_FIELD_MAC_ADDR_SIZE  6
#define EEPROM_FIELD_COUNTRY_CODE_SIZE  3

//format of common part of eeprom
typedef struct
{
    //always ensure fields are aligned to 32-bit boundaries
    tANI_U16  productId;
    tANI_U8   eepImageVersion;
    tANI_U8   eepSize;

    tANI_U8   cardType;
    tANI_U8   numOfTxChains;
    tANI_U8   numOfRxChains;
    tANI_U8   productBands;

    tANI_U8   padcGain_24ghz;
    tANI_U8   unused1[3];

    tANI_U8   sdramInfo;
    tANI_U8   sdramReserved[3];

    tANI_U8   macAddr[EEPROM_FIELD_MAC_ADDR_SIZE];
    tANI_U8   unused2[2];
		
    char      countryCode[EEPROM_FIELD_COUNTRY_CODE_SIZE];        // Country Code
    tANI_U8   defaultRegDomain;      // must match the country code

    tANI_U8   numStations;
    tANI_U8   unused3[3];

	tANI_U32  cksumPreceding;
}sEepromFields;

typedef struct
{
    tANI_U16 tableID;
    tANI_U16 tableOffset;
    tANI_U16 numOfEntries;
    tANI_U16 sizeOfEntry;
}sEepromTableDir;

typedef struct
{
    sEepromFields fields;
    tANI_U32  cksumTables;
    sEepromTableDir tableDirEntry[MAX_EEPROM_TABLE];
}sHalEeprom;


#define EEPROM_TABLE_DIR_SIZE   (MAX_EEPROM_TABLE * sizeof(sEepromTableDir))


#endif

