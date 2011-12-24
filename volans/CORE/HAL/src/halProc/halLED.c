/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halLED.c:  Handles LED feature.
 * Author:    Naveen G
 * Date:      06/27/2007
 *
 * --------------------------------------------------------------------------
 */
#include "halInternal.h"
#include "halLED.h"
#include "halUtils.h"
#include "halDebug.h"

/** Definitions for LED.*/
#define HAL_LED0_MASK                    0x1
#define HAL_LED1_MASK                    0x2

/** Number tx and rx packets to trigger LED Activity.*/
#define HAL_LED_THRESHOLD	             50

#define VENDOR_CARDBUS_INDEX             0
#define VENDOR_PCI_INDEX                 1
#define VENDOR_PCIE_INDEX                2

static const tHalLedConfig gVendorLedInfo[] = {
/*   SID      SVID   CARDTYPE idleOnTh OffTh scanOnTh OffTh trOnTh OffTh singleLed  pwInd         trInd        scanInd      Enabled/Disabled    Polarity-Invert*/
/* VENDOR_CARDBUS_DEFAULT*/
    {0xffff, 0xffff, CARD_TYPE_CARDBUS, 1, 10, 1, 1, 2, 1,   eANI_BOOLEAN_FALSE, eHAL_LED_ON,    eHAL_LED_ON, eHAL_LED_OFF, eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_FALSE},
/* VENDOR_PCI_DEFAULT*/
    {0xffff, 0xffff, CARD_TYPE_MINIPCI, 0, 0, 1, 1, 2, 1,    eANI_BOOLEAN_TRUE,  eHAL_LED_ON,    eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_TRUE},
/* VENDOR_PCIe_DEFAULT */
    {0xffff, 0xffff, CARD_TYPE_MINIPCIX, 0, 0, 1, 1, 2, 1,   eANI_BOOLEAN_TRUE,  eHAL_LED_ON,    eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_TRUE},
/* VENDOR_PCIe_ASUS */
    {0x1043, 0x156f, CARD_TYPE_MINIPCIX, 0, 0, 1, 1, 2, 1,   eANI_BOOLEAN_TRUE,  eHAL_LED_OFF,   eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_TRUE},
/* VENDOR_PCIe_LENOVO */
    {0x17aa, 0x0003, CARD_TYPE_MINIPCIX, 1, 4, 1, 1, 2, 1,   eANI_BOOLEAN_TRUE,  eHAL_LED_ON,    eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_TRUE},
/* VENDOR_PCIe_DELL */
    {0x1028, 0x0003, CARD_TYPE_MINIPCIX, 0, 0, 1, 1, 2, 1,   eANI_BOOLEAN_TRUE,  eHAL_LED_BLINK, eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_TRUE,  eANI_BOOLEAN_TRUE},
/* VENDOR_PCIe_APPLE @TODO SID/SVID */
    {0xffff, 0xffff, CARD_TYPE_MINIPCIX, 0, 0, 1, 1, 2, 1,   eANI_BOOLEAN_TRUE,  eHAL_LED_BLINK, eHAL_LED_ON, eHAL_LED_ON,  eANI_BOOLEAN_FALSE, eANI_BOOLEAN_TRUE},
    
};

static tANI_U32 __halGetVendorIndex(tpAniSirGlobal pMac, tANI_U32 cardType);

/**
 *    @brief : Initialize the LED.
 *    
 *    @param pMac  an instance of MAC parameters
 *    @return None
 */
eHalStatus halInitLed(tpAniSirGlobal pMac)
{
    tANI_U32 vendorIdx;
    tANI_U32 cardtype;
    
    /** Get card type.*/
    cardtype = halUtil_GetCardType(pMac); 
    /** Get Vendor Info */
    vendorIdx = __halGetVendorIndex(pMac, cardtype);
    palZeroMemory(pMac->hHdd, &pMac->hal.ledParam, sizeof(tHalLedParam));
    pMac->hal.ledParam.config = gVendorLedInfo[vendorIdx];

    if ((pMac->hal.ledParam.config.powerInd == eHAL_LED_ON) &&
          (pMac->hal.ledParam.config.bEnable == eANI_BOOLEAN_TRUE))
        halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_ON);

    return eHAL_STATUS_SUCCESS;
}

/**
 *    @brief : Uninitialize the LED and LED Activity Timer.
 *    
 *    @param pMac  an instance of MAC parameters
 *    @return None
 */
eHalStatus halCloseLed(tpAniSirGlobal pMac)
{
    if (pMac->hal.ledParam.config.bEnable == eANI_BOOLEAN_TRUE)
    {
        halSetLed(pMac, eHAL_LINK_LED, eHAL_LED_OFF); // Link LED off
        halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_OFF); //Traffic LED off
    }
    
    return eHAL_STATUS_SUCCESS;
}

/**
 *    @brief : Sets GPIO LED to on or off based upon the LED Type.
 *
 *    @note  : We strongly assume that this is the only function accessing LED GPIO
 *    register!!!
 *
 *    @param pMac  an instance of MAC parameters
 *    @param ledType its either Traffic or the Link
 *    @param ledState its either LedOn or LedOff
 *    @return None
 */
void halSetLed(tpAniSirGlobal pMac, tHalLedType ledType, tHalLedState ledState)
{
    tANI_U32 ledMask;
    tANI_U32 ledGPIOReg;

    ledGPIOReg = pMac->hal.ledParam.ledGPIOReg;

    if (pMac->hal.ledParam.config.singleLed == eANI_BOOLEAN_TRUE)
         ledMask = HAL_LED0_MASK;
    else
    {
        if (ledType == eHAL_LINK_LED) /* LINK LED */
            ledMask = HAL_LED1_MASK;
        else /* TRAFFIC POWER and SCAN */
            ledMask = HAL_LED0_MASK;
    }

    if (pMac->hal.ledParam.config.bInvert == eANI_BOOLEAN_TRUE)
    {
        if(ledState == eHAL_LED_ON)
            ledState = eHAL_LED_OFF;
        else
            ledState = eHAL_LED_ON;
    }
    
    if (ledState == eHAL_LED_ON)
        ledGPIOReg |= ledMask;
    else
        ledGPIOReg &= (~ledMask);

    if (halWriteRegister(pMac, GPIO_LED_REG, ledGPIOReg) != eHAL_STATUS_SUCCESS)
        return;

    pMac->hal.ledParam.ledGPIOReg = ledGPIOReg;
}

/**
 *    @brief :Scan LED algorithm is implemented based on the Mac entering into scan mode,
 *    the behaviour is slow blink.
 *
 *    @param pMac A pointer to the tAniSirGlobal structure
 *    @return None
 */
static void __halScanLedHandler(tpAniSirGlobal pMac)
{
    if (pMac->hal.ledParam.linkLedOn == eANI_BOOLEAN_TRUE)
        return;

    if (pMac->hal.scanParam.isScanInProgress == eANI_BOOLEAN_TRUE)
    {
        if (pMac->hal.ledParam.scanIndTriggered == eANI_BOOLEAN_FALSE)
        {
            if (pMac->hal.ledParam.config.powerInd == eHAL_LED_ON)
                pMac->hal.ledParam.OnCnt = pMac->hal.ledParam.config.scanOnTh;
            else
                pMac->hal.ledParam.OnCnt = 0;

            pMac->hal.ledParam.OffCnt = 0;
            pMac->hal.ledParam.scanIndTriggered = eANI_BOOLEAN_TRUE;
        }

        if (pMac->hal.ledParam.OnCnt == 0)
        {
            halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_ON);  //turn On LED
            pMac->hal.ledParam.OnCnt++;
        }
        else
        {
            pMac->hal.ledParam.OnCnt++;
            if (pMac->hal.ledParam.OnCnt > pMac->hal.ledParam.config.scanOnTh)
            {
                if (pMac->hal.ledParam.OffCnt == 0)
                {
                    halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_OFF);   // turn off LED
                    pMac->hal.ledParam.OffCnt = pMac->hal.ledParam.config.scanOffTh;
                }
                else
                {
                    pMac->hal.ledParam.OffCnt--;
                    if (pMac->hal.ledParam.OffCnt == 0)
                        pMac->hal.ledParam.OnCnt = 0;
                }
            }
        }
    } 
    /** Restore the Power indication state after Scan Indication has happened, as 
        the same LED is used to indicate Scan and Power.*/
    else if (pMac->hal.ledParam.scanIndTriggered == eANI_BOOLEAN_TRUE)
    {
        if (pMac->hal.ledParam.config.powerInd == eHAL_LED_ON)
            halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_ON);
        else if(pMac->hal.ledParam.config.powerInd == eHAL_LED_OFF)
            halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_OFF);

        pMac->hal.ledParam.scanIndTriggered = eANI_BOOLEAN_FALSE;         
    }
}

/**
 *    @brief : Power LED handler is for the the LED indication on power up and not associated
 *    condition, where LED should blink periodically to indicate that the card is powered or should
 *    be in a stable non flashing state.
 *
 *    @param pMac A pointer to the tAniSirGlobal structure
 *    @return None
 */
static void __halPowerLedHandler(tpAniSirGlobal pMac)
{
    tRxpMode mode;
    mode = halRxp_getRxpMode(pMac);

    if (mode == eRXP_IDLE_MODE && pMac->hal.scanParam.isScanInProgress != eANI_BOOLEAN_TRUE)
    {
        if (pMac->hal.ledParam.OnCnt == 0)
        {
            halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_ON);
            pMac->hal.ledParam.OnCnt++;
        }
        else
        {
            pMac->hal.ledParam.OnCnt++;
            if (pMac->hal.ledParam.OnCnt > pMac->hal.ledParam.config.idleOnTh)
            {
                if (pMac->hal.ledParam.OffCnt == 0)
                {
                    halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_OFF);   // turn off LED
                    pMac->hal.ledParam.OffCnt = pMac->hal.ledParam.config.idleOffTh;
                }
                else
                {
                    pMac->hal.ledParam.OffCnt--;
                    if (pMac->hal.ledParam.OffCnt == 0)
                        pMac->hal.ledParam.OnCnt = 0;
                }
            }
        }
    }
}


/**
 *    @brief :Traffic LED algorithm is implemented based on the transmitted and
 *    received packets to and from PHY respectively. This controls the traffic
 *    LED on/off.
 *
 *    @param pMac A pointer to the tAniSirGlobal structure
 *    @return None
 */
static void __halTrafficLedHandler(tpAniSirGlobal pMac)
{
    tANI_U32 prevTx, curTx;
    tANI_U32 prevRx, curRx;
    tANI_U32 pktCnt;

    prevTx = curTx = 0;
    prevRx = curRx = 0;

    if (pMac->hal.ledParam.trafficOnCnt < pMac->hal.ledParam.config.trafficOnTh) {
        /*
         * Determine the number of transmitted frames since the last time the
         * LED was turned on.
         */
        prevTx = pMac->hal.ledParam.prevTxNum;
        curTx = pMac->hal.halMac.macStats.txCount;
        /*
         * Determine the number of received frames since the last time the
         * LED was turned on.
         */
        prevRx = pMac->hal.ledParam.prevRxNum;
        curRx = pMac->hal.halMac.macStats.rxCount;
    }

    pktCnt = (tANI_U32) (curTx - prevTx);
    pktCnt += (tANI_U32) (curRx - prevRx);

    if (pktCnt > HAL_LED_THRESHOLD) {
        // turn on LED
        if (pMac->hal.ledParam.trafficOnCnt == 0 && pMac->hal.ledParam.config.singleLed != eANI_BOOLEAN_TRUE)
            halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_ON);     // turn on LED

        pMac->hal.ledParam.prevTxNum = curTx;
        pMac->hal.ledParam.prevRxNum = curRx;
        pMac->hal.ledParam.trafficOnCnt++;
    } else {
        // turn off LED
        if (pMac->hal.ledParam.trafficOnCnt > 0) {
            if (pMac->hal.ledParam.trafficOffCnt == 0){
                halSetLed(pMac, eHAL_TRAFFIC_LED, eHAL_LED_OFF);   // turn off LED
                pMac->hal.ledParam.trafficOffCnt = pMac->hal.ledParam.config.trafficOffTh;
            } else {
                pMac->hal.ledParam.trafficOffCnt--;

                if (pMac->hal.ledParam.trafficOffCnt == 0)
                {
                    pMac->hal.ledParam.trafficOnCnt = 0;
                    /* Incase of Single LED make sure you turn on LED when no Traffic to indicate
                       link State.*/
                    if (pMac->hal.ledParam.linkLedOn == eANI_BOOLEAN_TRUE && 
                                       pMac->hal.ledParam.config.singleLed == eANI_BOOLEAN_TRUE) 
                        halSetLed(pMac, eHAL_LINK_LED, eHAL_LED_ON);
                }
            }
        }
    }

    return;
}

/**
 *    @brief : LED handler handles, Scan Indication, Idle Mode blinking Indication and
 *    traffic Indication.
 *
 *    @param pMac A pointer to the tAniSirGlobal structure
 *    @return None
 */
void halLedHandler(tpAniSirGlobal pMac)
{
    /** If Power Indication required.*/
    if (pMac->hal.ledParam.config.powerInd == eHAL_LED_BLINK)
        __halPowerLedHandler(pMac);

    /** If Scan Indication required.*/
    if (pMac->hal.ledParam.config.scanInd == eHAL_LED_ON)
        __halScanLedHandler(pMac);

    /** If Traffic Indication required.*/
    if (pMac->hal.ledParam.config.trafficInd == eHAL_LED_ON)
        __halTrafficLedHandler(pMac);

    return;
}

/** ------------------------------------------------------
\fn      halGetVendorIndex
\brief   This function gets the vendor index from 
\        SID/SVID info read from eeprom.
\param   tpAniSirGlobal  pMac
\param   tANI_U32 cardType
\return  tANI_U32 vendor Index
\ -------------------------------------------------------- */
static tANI_U32 __halGetVendorIndex(tpAniSirGlobal pMac, tANI_U32 cardType)
{
    tANI_U32 i;
    tANI_U16 subsystemId, eepromSID;
    tANI_U16 subsystemVendorId, eepromSVID;    
    uEepromFields eepromData;
    tANI_U32 vendorIdx=VENDOR_CARDBUS_INDEX; /** defaults to Cardbus*/
    
    switch (cardType)
    {
        case CARD_TYPE_APX:
        case CARD_TYPE_MINIPCI:
            vendorIdx = VENDOR_PCI_INDEX;
        case CARD_TYPE_CARDBUS:
            eepromSID = EEPROM_PCI_SUBSYSTEM_ID;
            eepromSVID = EEPROM_PCI_SUBSYSTEM_VENDOR_ID;
            break;
        case CARD_TYPE_MINIPCIX:
        case CARD_TYPE_PCIX:
            vendorIdx = VENDOR_PCIE_INDEX;
            eepromSID = EEPROM_PCIE_SUBSYSTEM_ID;
            eepromSVID = EEPROM_PCIE_SUBSYSTEM_VENDOR_ID;
            break;
        case CARD_TYPE_USB:
        default:
            return vendorIdx;
    }

    if (halReadEepromField((tHalHandle) pMac, eepromSID, &eepromData) != eHAL_STATUS_SUCCESS)
    {
        halLog(pMac, LOGE, "Error Reading SID EEPROM Field\n");
        return vendorIdx;
    }
    subsystemId = eepromData.subsystemId;
    if (halReadEepromField((tHalHandle) pMac, eepromSVID, &eepromData) != eHAL_STATUS_SUCCESS)
    {
        halLog(pMac, LOGE, "Error Reading SVID EEPROM Field\n");
        return vendorIdx;
    }
    subsystemVendorId = eepromData.subsystemVendorId;

    for (i=0; i<sizeof(gVendorLedInfo)/sizeof(gVendorLedInfo[0]); i++)
    {
        if ((gVendorLedInfo[i].sid == subsystemId) && 
             (gVendorLedInfo[i].svid == subsystemVendorId) && (gVendorLedInfo[i].cardType == cardType))
            return i;
    }

    return vendorIdx;
}
