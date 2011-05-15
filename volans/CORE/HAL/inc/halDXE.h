/**
 *
 *  @file:      halDXE.h
 *
 *  @brief:     Provides all the APIs to configure DXE.
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------------------------------
 * 04/26/2007  Naveen G			File created.
 * 06/18/2008  Sanoop Kottontavida	Added halDXE_Open/Close/Start/Stop
 * 								and required macros.
 */

#ifndef _HALDXE_H_
#define _HALDXE_H_

#include "halTypes.h"
#include "aniGlobal.h"

#define DXE_CH_REG_SIZE        0x40
#define DXE_CH_CTRL_REG        0x0000
#define DXE_CH_STATUS_REG      0x0004
#define DXE_CH_SZ_REG          0x0008
#define DXE_CH_SADRL_REG       0x000C
#define DXE_CH_SADRH_REG       0x0010
#define DXE_CH_DADRL_REG       0x0014
#define DXE_CH_DADRH_REG       0x0018
#define DXE_CH_DESCL_REG       0x001C
#define DXE_CH_DESCH_REG       0x0020
#define DXE_CH_LST_DESCL_REG   0x0024
#define DXE_CH_LST_DESCH_REG   0x0028
#define DXE_CH_BD_REG          0x002C
#define DXE_CH_HEAD_REG        0x0030
#define DXE_CH_TAIL_REG        0x0034
#define DXE_CH_PDU_REG         0x0038
#define DXE_CH_TSTMP_REG       0x003C


// TransferType

#define DXE_XFR_HOST_TO_HOST    0
#define DXE_XFR_BMU_TO_BMU      1
#define DXE_XFR_HOST_TO_BMU     2
#define DXE_XFR_BMU_TO_HOST     3

#define NOVA_DXE_CH_REG_SIZE        0x40

#define NOVA_DXE_CH_CTRL_REG        0x0000
#define NOVA_DXE_CH_STATUS_REG      0x0004
#define NOVA_DXE_CH_SZ_REG          0x0008
#define NOVA_DXE_CH_SADRL_REG       0x000C
#define NOVA_DXE_CH_SADRH_REG       0x0010
#define NOVA_DXE_CH_DADRL_REG       0x0014
#define NOVA_DXE_CH_DADRH_REG       0x0018
#define NOVA_DXE_CH_DESCL_REG       0x001C
#define NOVA_DXE_CH_DESCH_REG       0x0020
#define NOVA_DXE_CH_LST_DESCL_REG   0x0024
#define NOVA_DXE_CH_LST_DESCH_REG   0x0028
#define NOVA_DXE_CH_BD_REG          0x002C
#define NOVA_DXE_CH_HEAD_REG        0x0030
#define NOVA_DXE_CH_TAIL_REG        0x0034
#define NOVA_DXE_CH_PDU_REG         0x0038
#define NOVA_DXE_CH_TSTMP_REG       0x003C

// These are the original bit masks of DXE Descriptor Ctrl Word definition

#define DXE_DESC_CTRL_VALID         0x00000001
#define DXE_DESC_CTRL_XTYPE_MASK    0x00000006      // 0000-0110
#define DXE_DESC_CTRL_XTYPE_H2H     0x00000000      // 0000-0000
#define DXE_DESC_CTRL_XTYPE_B2B     0x00000002      // 0000-0010
#define DXE_DESC_CTRL_XTYPE_H2B     0x00000004      // 0000-0100
#define DXE_DESC_CTRL_XTYPE_B2H     0x00000006      // 0000-0110
#define DXE_DESC_CTRL_EOP           0x00000008
#define DXE_DESC_CTRL_BDH           0x00000010
#define DXE_DESC_CTRL_SIQ           0x00000020
#define DXE_DESC_CTRL_DIQ           0x00000040
#define DXE_DESC_CTRL_PIQ           0x00000080
#define DXE_DESC_CTRL_PDU_REL       0x00000100
#define DXE_DESC_CTRL_BTHLD_SEL     0x00001E00
#define DXE_DESC_CTRL_PRIO          0x0000E000
#define DXE_DESC_CTRL_STOP          0x00010000
#define DXE_DESC_CTRL_INT           0x00020000
#define DXE_DESC_CTRL_DFMT          0x10000000
#define DXE_DESC_CTRL_RSVD          0xfffc0000

#define DXE_PHY_ADDR_MASK_UPPER_3BITS	0x1FFFFFFF

/** DXE HW Long Descriptor format */
typedef struct tDXELongDesc
{
	tANI_U32 srcMemAddrL;
    tANI_U32 srcMemAddrH;
    tANI_U32 dstMemAddrL;
    tANI_U32 dstMemAddrH;
    tANI_U32 phyNextL;
    tANI_U32 phyNextH;
} DXELongDesc;


/** DXE HW Short Descriptor format */
typedef struct tDXEShortDesc
{
	tANI_U32 srcMemAddrL;
    tANI_U32 dstMemAddrL;
    tANI_U32 phyNextL;
} DXEShortDesc;

/** DXE HW Descriptor */
typedef struct tagDXEDesc
{
    //DESC_CTRL
    union
    {
        tANI_U32 ctrl;

        tANI_U32 valid               :1;     //0 = DMA stop, 1 = DMA continue with this descriptor
        tANI_U32 transferType        :2;     //0 = Host to Host space
                                             //1 = BMU to BMU space
                                             //2 = Host to BMU space
                                             //3 = BMU to Host space
        tANI_U32 eop                 :1;     //End of Packet
        tANI_U32 bdHandling          :1;          //if transferType = Host to BMU, then 0 means first 128 bytes contain BD, and 1 means create new empty BD
                                             //if transferType = BMU to Host, then 0 means insert BD in front of payload, and 0 discard BD
        tANI_U32 siq                 :1;     // SIQ
        tANI_U32 diq                 :1;     // DIQ
        tANI_U32 pduRel              :1;     //0 = don't release BD and PDUs when done, 1 = release them
        tANI_U32 bthldSel            :4;     //BMU Threshold Select
        tANI_U32 prio                :3;     //Specifies the priority level to use for the transfer
        tANI_U32 stopChannel         :1;     //1 = DMA stops processing further, channel requires re-enabling after this
        tANI_U32 intr                :1;     //Interrupt on Descriptor Done
        tANI_U32 rsvd                :1;     //reserved
        tANI_U32 transferSize        :14;    //14 bits used - ignored for BMU transfers, only used for host to host transfers?
    } cw;

    tANI_U32 xfrSize;

    union
    {
    	DXELongDesc		dxe_long_desc;
    	DXEShortDesc	dxe_short_desc;
    }dxedesc;
    
} sDXEDesc;

// Descriptor Control Block

typedef struct tagDCB
{
    struct tagDCB   *next;              // Next pointer
    struct tagDCB   *nextFrame;         // Ptr to the 1st DCB of the next frame (only used in 1st DCB of a frame)
    struct tagDCB   *lastDesc;          // Ptr to the last DCB of this frame (ditto)
    void            *xfrFrame;          // Back ptr to the frame passed from HAL
    tANI_U32        nDescs;             // Number of descriptors in this frane (only vaild in 1st DCB of a frame)
    void            *virtAddr;          // The DXE implementation will have to use the virtual memory pointer to reference the descriptor memory to set the fields
    tANI_U32        physAddr;           // This is the physical memory address of the descriptor, so we know what to plug into the descriptor address contents
} sDCB;

typedef enum eDXEQueueMethod
{
    DXE_QUEUE_LINEAR,
    DXE_QUEUE_CIRCULAR
}eDXEQueueMethod;

typedef struct tagDxeCCB                // One per DXE channel
{
    sDCB            *xfrDescHead;       // List head & tail for List based DXE decsritpor
    sDCB            *xfrDescTail;
    sDCB            *pRingFreeHead;     // Ring head for Ring based DXE decsritpor
    tANI_U32        noXfrDescUsed;      // No. of xfr desc in chain
    tANI_U32        nDescs;             // Number of URBs for USB or descriptors for DXE that can be queued for transfer at one time
    tANI_U32        nFreeDescs;         // Number of free descriptors.
    tANI_U32        nRxBuffers;         // Maximum number of receive buffers  of shared memory to use for this pipe
    tANI_U32        refWQ;
    tANI_U32        refWQ_swapped;
    tANI_U32        refEP;
    tANI_U32        bmuThdSel;
    tANI_U32        xfrType;
    tANI_U32        chDXEBaseAddr;
    tANI_U32        sdioRxDescStart;
#ifdef WLAN_HAL_VOLANS
    tANI_U32        sdioRxHiDescStart;
#endif
    tANI_U32        sdio_ch_desc;
    sDXEDesc        sdioDesc;
    eDXEQueueMethod queueMethod;
    tANI_U32        chPriority;
	// The following call back is commented out for the time being and need to be confirmed as palPipes.h file is no more used.
    // palXfrCompleteFPtr callback;        // Callback when transfer is complete, if not NULL
    tANI_BOOLEAN    bdPresent;          // 1 = BD attached to frames for this pipe
    tANI_BOOLEAN    chEnabled;
    tANI_BOOLEAN    chConfigured;    
    tANI_U32        chk_size;
    eDMAChannel     channel;
    tANI_U32        chDXEStatusRegAddr;
    tANI_U32        chDXEDesclRegAddr;
    tANI_U32        chDXEDeschRegAddr;
	tANI_U32        chDXELstDesclRegAddr;
    tANI_U32        chDXECtrlRegAddr;
    tANI_U32        chDXESzRegAddr;
    tANI_U32        chDXEDadrlRegAddr;
    tANI_U32        chDXEDadrhRegAddr;
    tANI_U32        chk_size_mask;
    tANI_U32        bmuThdSel_mask;
    tANI_U32        cw_ctrl_read;
    tANI_U32        cw_ctrl_write;
    tANI_U32        cw_ctrl_write_valid;
    tANI_U32        cw_ctrl_write_eop_int;
    tANI_U32        chan_mask;
    tANI_U32        chDXECtrlRegValue;

    tANI_BOOLEAN	use_lower_4g;		/**< Added in Gen5 for Prefetch */
    tANI_BOOLEAN    use_short_desc_fmt;   
} sDxeCCB;

// DXE Global Control Block

typedef struct sAniHalDxe
{
    tANI_U32        sdioRxDescStart;         // DXE Rx descriptor in device memory 
#ifdef WLAN_HAL_VOLANS
    tANI_U32        sdioRxHiDescStart;         // DXE Rx Hi descriptor in device memory
#endif
    // DXE Channel Control Blocks

    sDxeCCB         DxeCCB[MAX_DXE_CHANNEL];

    tANI_BOOLEAN    Configured;
} tAniHalDxe, *tpAniHalDxe;


typedef struct sDxeChannelCfg
{
    eDXEQueueMethod queueMethod;
    tANI_U32 nDescs;            // Number of URBs for USB or descriptors for DXE that can be queued for transfer at one time
    tANI_U32 nRxBuffers;        // Maximum number of receive buffers  of shared memory to use for this pipe
    tANI_U32 refWQ;             // Reference WQ - for H2B and B2H only
    tANI_U32 refEP;             //for usb only, endpoint info for CH_SADR or CH_DADR
    tANI_U32 xfrType;           // H2B(Tx), B2H(Rx), H2H(SRAM<->HostMem R/W)
    tANI_U32 chPriority;        // Channel Priority 7(Highest) - 0(Lowest)
    tANI_BOOLEAN bdPresent;     // 1 = BD attached to frames for this pipe
    // palPipes.h is not used and hence the following commented out line need to be reviewed.
    //palXfrCompleteFPtr callback;//callback when transfer is complete, if not NULL
    tANI_U32 chk_size;
    tANI_U32 bmuThdSel;

    tANI_BOOLEAN	useLower4G;	/**< Added in Gen5 for Prefetch */
    tANI_BOOLEAN  useshortdescfmt;    
}sDXEChannelCfg;

/** DXE Global Configuration */
typedef struct
{
	tANI_U32 nDescs;                    //number of URBs for USB or descriptors for DXE that can be queued for transfer at one time
    tANI_U32 nRxBuffers;                //maximum number of receive buffers from physical memory to use for this pipe
    tANI_BOOLEAN preferCircular;        //1 = use circular descriptor handling if available, linear otherwise
    tANI_BOOLEAN bdPresent;             //1 = BD attached to frames for this pipe
    // Need to revisit the following two lines that are commented out since palPipes.h is removed.
    //palXfrCompleteFPtr callback;        //callback when transfer is complete, if not NULL
	//palXfrCompleteFPtr rxCallback;      //Rx callback when transfer type is H2H, if not NULL
    tANI_U32 refWQ;             // Reference WQ - for H2B and B2H only
    tANI_U32 xfrType;           // H2B(Tx), B2H(Rx), H2H(SRAM<->HostMem R/W)
    tANI_U32 chPriority;        // Channel Priority 7(Highest) - 0(Lowest)
    tANI_BOOLEAN bCfged;        //whether the pipe has been configured
    tANI_U32 indexEP;     //This is for USB only, it is the index of TX/RX endpoint, TX_DATA_PIPE_OUT...
    tANI_U32 bmuThreshold; //BMU threshold 
	// For PAL's internal use
	void *pReserved1;
	void *pReserved2;

	tANI_BOOLEAN	use_lower_4g;	/**< Added for Gen5 Prefetch */
	tANI_BOOLEAN	use_short_desc_fmt;
} sDXEConfig;


eHalStatus halIntDXEErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource);

/**
 * Allocates the resources required for DXE module.
 */
eHalStatus halDXE_Open(tHalHandle hHalHandle, void *arg);

/**
 * Deallocates the resources used by DXE module.
 */
eHalStatus halDXE_Close(tHalHandle hHalHandle, void *arg);

/**
 * @fn	halDXE_Start
 * @brief	Configures 2 DMA channels. One for Tx and one for Rx.
 *
 * @param	hHalHandle	Global reference to HAL module
 * @param	pDXECfg		Pointer to a structure where all the global
 * 						configurations specific to DXE are stored.
 *						Caller uses it to configure the DMA channels.
 * @return	eHalStatus	Success or Failure
 */
//eHalStatus halDXE_Start(tHalHandle hHalHandle, sDXEConfig *pDXECfg);
eHalStatus halDXE_Start(tHalHandle hHalHandle,  void *arg);


/**
 * Uninitializes the DXE module.
 */
eHalStatus halDXE_Stop(tHalHandle hHalHandle, void *arg);

/* 
 * Ensure DXE IDLE state by stopping the Rx channel
 */
eHalStatus halDxe_EnsureDXEIdleState(tHalHandle hHalHandle);

/* 
 * Enable/Disable the DXE 
 */
eHalStatus halDxe_EnableDisableDXE(tHalHandle hHalHandle, tANI_U8 enable);

/*
 * Get DxE channel status
 */
VOS_STATUS halDxe_DxeChannelIdleStatus(tANI_U32 *pStatus, v_PVOID_t pMacContext);

#endif /* _HALDXE_H_ */



