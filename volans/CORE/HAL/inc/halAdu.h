/**
 *
 *  @file:         halAdu.h
 *
 *  @brief:        Provides all the MAC APIs to the ADU Hardware Block.
 *
 *  @author:        Madhava Reddy S
 *
 *  Copyright (C) 2008, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/09/2007  File created.
 * 12/04/2007  MIMO PS tables changes made.
 */
#ifndef _HALADU_H_
#define _HALADU_H_

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h" // tSirMacAddr

#define  HAL_ADU_REG_RECFG_NO_WAIT       0
#define  HAL_ADU_REG_RECFG_DEFAULT_WAIT  10

#define  HAL_ADU_REG_RECFG_INIT_CMD      0x0
#define  HAL_ADU_REG_RECFG_WAIT_CMD      0x80
#define  HAL_ADU_REG_RECFG_TBL_END_CMD   0xFF

#define  HAL_ADU_REG_RECFG_ADDR_MASK        0x7FFFFFFF
#define  HAL_ADU_REG_RECFG_WAIT_TIME_MASK   0xFFFFFF
#define  HAL_ADU_REG_RECFG_TBL_END_CMD_MASK 0xFF000000

#ifdef ANI_SUPPORT_SMPS
#define  SPICA

// MIMO Power Save Mode
typedef enum
{
#ifdef SPICA
  eSIR_HT_MIMO_PS_2SS_1SS = 0, // Switch from 2 Spatial Stream to 1 Spatial Stream
  eSIR_HT_MIMO_PS_1SS_2SS = 1, // Switch from 1 Spatial Stream to 2 Spatial Stream
#else
  eSIR_HT_MIMO_PS_2SS_1SS = 0, // Switch from 2 Spatial Stream to 1 Spatial Stream
  eSIR_HT_MIMO_PS_3SS_1SS = 1, // Switch from 3 Spatial Stream to 1 Spatial Stream
  eSIR_HT_MIMO_PS_4SS_1SS = 2, // Switch from 4 Spatial Stream to 1 Spatial Stream
  eSIR_HT_MIMO_PS_1SS_2SS = 3, // Switch from 1 Spatial Stream to 2 Spatial Stream
  eSIR_HT_MIMO_PS_1SS_3SS = 4, // Switch from 1 Spatial Stream to 3 Spatial Stream
  eSIR_HT_MIMO_PS_1SS_4SS = 5, // Switch from 1 Spatial Stream to 4 Spatial Stream
#endif //#ifdef SPICA

  eSIR_HT_MIMO_PS_MODE_MAX
} eSirMacHTMIMOPowerSaveMode;
#endif

typedef struct sHalRegCfg {
    tANI_U32    addr;
    tANI_U32    value;
} tHalRegCfg, *tpHalRegCfg;

typedef struct sHalMimoPsCfg {
    tANI_U32    size;
    tpHalRegCfg halRegCfgPtr;
} tHalMimoPsCfg, *tpHalMimoPsCfg;


#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack(push, 1)
#elif defined(__ANI_COMPILER_PRAGMA_PACK)
#pragma pack(1)
#else
#endif

typedef __ani_attr_pre_packed struct sAduUmaStaDesc {

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved1:4;
    tANI_U32 ackPolicy:8;
    /** DPU tag to be insert to the BD */
    tANI_U32 dpuSig:3;
    /** Valid Bit. This has to be set to '1' to indicate the entry is valid for frame translation */
    tANI_U32 valid:1;
    /** BSSID to be used for frame translation */
    tANI_U32 bssidHi:16;
#else
    tANI_U32 bssidHi:16;
    tANI_U32 valid:1;
    tANI_U32 dpuSig:3;
    tANI_U32 ackPolicy:8;
    tANI_U32 reserved1:4;
#endif

    /** BSSID to be used for frame translation */
    tANI_U32 bssidLo;

#ifdef ANI_BIG_BYTE_ENDIAN
    /** DPU descriptor index to be inserted to the BD */
    tANI_U32 dpuDescIdx:8;
    /** The Station ID to be inserted to the BD */
    tANI_U32 staIdx:8;
    tANI_U32 order :1;
    tANI_U32 wep :1;
    tANI_U32 moreData :1;
    tANI_U32 powerMgmt :1;
    tANI_U32 retry :1;
    tANI_U32 moreFrag :1;
    tANI_U32 fromDS :1;
    tANI_U32 toDS :1;

    tANI_U32 subType :4;
    tANI_U32 type :2;
    tANI_U32 protVer :2;
#else
    /** The frame control bit fields for wmac header */
    tANI_U32 protVer :2;
    tANI_U32 type :2;
    tANI_U32 subType :4;

    tANI_U32 toDS :1;
    tANI_U32 fromDS :1;
    tANI_U32 moreFrag :1;
    tANI_U32 retry :1;
    tANI_U32 powerMgmt :1;
    tANI_U32 moreData :1;
    tANI_U32 wep :1;
    tANI_U32 order :1;


    tANI_U32 staIdx:8;
    tANI_U32 dpuDescIdx:8;
#endif

} __ani_attr_packed tAduUmaStaDesc, *tpAduUmaStaDesc;

#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack(pop)
#elif defined(__ANI_COMPILER_PRAGMA_PACK)
#else
#endif

typedef struct sAduInfo {
    tANI_U8 staIdx;
    tANI_U8 valid;          /**< 0 means not valid */
} tAduInfo, *tpAduInfo;

#ifndef ANI_LITTLE_BIT_ENDIAN
#define WEP_IN_FRAME_CTL    0x0040
#define TODS_IN_FRAME_CTL   0x0001
#else
#define WEP_IN_FRAME_CTL    0x0002
#define TODS_IN_FRAME_CTL   0x0010
#endif

#define UMA_ACK_POLICY_UC_FRAME 0x00

#define ADU_MAX_DESCRIPTOR_ENTRIES 255

eHalStatus halAdu_Start(tHalHandle hHal, void *arg);
eHalStatus halAdu_UpdateControl(tpAniSirGlobal pMac, tANI_U32 mask);
eHalStatus halAdu_UpdateControlPushWq(tpAniSirGlobal pMac, tANI_U32 workQueue, tANI_U32 mask);
eHalStatus halAdu_UpdateRegRecfgTable(tpAniSirGlobal pMac, tANI_U32 regAddress, tANI_U32 regValue, tANI_U32 waitCycles );
eHalStatus halAdu_WriteUmaDefaultMacAddr(tpAniSirGlobal pMac, tANI_U32 macAddrLo, tANI_U32 macAddrHi, tANI_U8 staIdx);
eHalStatus halAdu_AddToUmaSearchTable(tpAniSirGlobal pMac, tANI_U32 macAddrLo, tANI_U32 macAddrHi, tANI_U8 staIdx);
eHalStatus halAdu_SetUmaStaDesc(tpAniSirGlobal pMac, tANI_U16 staId, tpAduUmaStaDesc pAduUmaStaDesc);


eHalStatus halAdu_UpdateUmaDescForPrivacy(tpAniSirGlobal pMac, tANI_U16 staIdx);

eHalStatus halAdu_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource);

eHalStatus halAdu_enableFrameTranslation(tpAniSirGlobal pMac);
eHalStatus halAdu_disableFrameTranslation(tpAniSirGlobal pMac);
eHalStatus halAdu_BckupUmaSearchTable(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halAdu_UpdateUMADefaultstaIdx(tpAniSirGlobal pMac,
    tANI_U8 staIdx);
eHalStatus halUMA_AllocId(tpAniSirGlobal pMac, tANI_U8 *umaIdx);

#if 0 //FIXME_NO_VIRGO
eHalStatus halAdu_SetMimoPScfg(tpAniSirGlobal pMac, eSirMacHTMIMOPowerSaveMode mode);
eHalStatus halAdu_switchMimoPSmode(tpAniSirGlobal pMac, eSirMacHTMIMOPowerSaveMode mode);
#endif
#endif  /* _HALADU_H */
