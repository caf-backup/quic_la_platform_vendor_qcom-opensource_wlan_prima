/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halLogDump.h:  Provides APIs hal
 * Author:    Susan Tsao
 * Date:      02/06/2006
 *
 * --------------------------------------------------------------------------
 */

#ifndef HALLOGDUMP_H
#define HALLOGDUMP_H

/* The following are the defines for the dump 
 * command 93 which modifies the timeouts based 
 * on arg1 which is checked against these defines.
 */
#define DUMPCMD_UCAST_DATA_RECEPTION_TIMEOUT  0x0
#define DUMPCMD_BCAST_DATA_RECEPTION_TIMEOUT  0x1
#define DUMPCMD_BTQM_QEMPTY_TIMEOUT           0x2
#define DUMPCMD_SIF_UNFREEZE_TIMEOUT          0x3    
#define DUMPCMD_FRST_BEACON_TIMEOUT           0x4    
#define DUMPCMD_BPS_EARLY_MODE_TIMEOUT        0x5
#define DUMPCMD_BPS_TX_ACTIVITY_TIMEOUT       0x6

/* The following are the defines for the dump 
 * command 94 which modifies the sleeptimes based 
 * on arg1 which is checked against these defines.
 */
#define DUMPCMD_SLEEPTIME_OVERHEADS           0x0
#define DUMPCMD_FORCED_SLEEPTIME_OVERHEADS    0x1


void halDumpInit(tpAniSirGlobal pMac);

void halLog_deleteRxpEntry(tpAniSirGlobal pMac, tANI_U32 hi, tANI_U16 lo );
void halLog_addRxpEntry(tpAniSirGlobal pMac, tANI_U32 hi, tANI_U16 lo, tANI_U32 staid, tANI_U32 role );
eHalStatus halLog_setBmuBdPduThreshold(tpAniSirGlobal pMac, tANI_U32 index, tANI_U16 numOfBd, tANI_U16 numOfPdu);
void halLog_dumpBmuWqInfo(tpAniSirGlobal pMac);

/* Functions to send message to MCPU */
eHalStatus halLog_sendDeleteBss(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 bssid);
eHalStatus halLog_sendInitScan(tpAniSirGlobal pMac, tANI_U32 sendMgmtFrame);
eHalStatus halLog_setScanMode(tpAniSirGlobal pMac);
eHalStatus halLog_setNormalMode(tpAniSirGlobal pMac);
void halLog_updateDbgControlRateTable(tpAniSirGlobal pMac);


/* Functions to print information */
void halLog_printMemoryMap(tpAniSirGlobal pMac);
void halLog_printRxpTableEntry(tpAniSirGlobal pMac, tANI_U8 lowIndex, tANI_U8 highIndex);
void halLog_printRxpBinarySearchTable(tpAniSirGlobal pMac);
eHalStatus halLog_printMtu(tpAniSirGlobal pMac);
void halLog_printStaTable(tpAniSirGlobal pMac);
eHalStatus halLog_printDpuDescriptor(tpAniSirGlobal pMac, tANI_U32 index);
void halLog_dumpWMMProfile(tpAniSirGlobal pMac);
eHalStatus halLog_printMcpu2HostMemory(tpAniSirGlobal pMac);
eHalStatus halLog_printHost2McpuMemory(tpAniSirGlobal pMac);

/* Functions to change Rate specific config for STA */
void  halRate_changeStaRate(tpAniSirGlobal pMac, tANI_U32 staid, tANI_U32 chnl, tHalMacRate halPriRateIdx, tHalMacRate halSecRateIdx,  tHalMacRate halTerRateIdx );
eHalStatus halRate_sendStaRateInfoMsg(tHalHandle hHal, tANI_U32 startStaIdx, tANI_U32 staCount, tANI_U32 nextReportPktCount, tANI_U32 nextReportMsec);
void  halRate_updateProtInfo(tpAniSirGlobal pMac,tANI_U32 staid, tANI_U32 chnl, tANI_U32 rateSelect, tANI_U32 halDataRateIdx, tANI_U32 *forceProtPolicy, tANI_U32 *forceProtRate);
void  halRateDbg_change11nRatePktType(tpAniSirGlobal pMac,tANI_U32 mixedMode, tANI_U32 startRate,  tANI_U32 endRateIdx);
void  halMacRaDumpHalRateTable(tpAniSirGlobal pMac);
void  halMacRaDumpStaRateInfo(tpAniSirGlobal pMac, tANI_U32 startStaIdx, tANI_U32 endStaIdx);
void  halMacRaDumpHalSamplingRateTable(tpAniSirGlobal pMac, tANI_U32 bssIdx, tANI_U32 minHigherSamplingRates, tANI_U32 maxHigherSamplingRates);
void  halMacRaSetHalAutoSamplingRateTable(tpAniSirGlobal pMac,tANI_U32 bssIdx, tHalMacRate halRateIdx, tANI_U32 samplingRateIdx0_3,tANI_U32 samplingRateIdx4_7,tANI_U32 samplingRateIdx8_11);
void  halMacRaDumpStaAllSupporetedRates(tpAniSirGlobal pMac, tANI_U16 staidx);



/* Other Functions */
void halLog_getCfg(tpAniSirGlobal pMac, tANI_U16 cfgId);
void halLog_setCfg(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 val);
void halLog_getStaSignature(tpAniSirGlobal pMac, tANI_U8 staIdx);

void halLog_memDump(tpAniSirGlobal pMac, const unsigned char *buffer, unsigned int len );
void halLog_dumpPsInfo(tpAniSirGlobal pMac);

// XIF-PHY register dump information
eHalStatus asicXIFReadPhyReg(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pData);
eHalStatus asicXIFWritePhyReg(tHalHandle hMac, tANI_U32 addr, tANI_U32 data);

/* LogDump specific changes */
char *dump_hal_print_adaptive_threshold( tpAniSirGlobal pMac, tANI_U32 arg1,
										 	    tANI_U32 arg2, tANI_U32 arg3,
										 		tANI_U32 arg4, char *p );

char *dump_hal_mpi_pmi_stats( tpAniSirGlobal pMac, tANI_U32 arg1,
									tANI_U32 arg2, tANI_U32 arg3,
									tANI_U32 arg4, char *p );

char *dump_hal_radar_regs( tpAniSirGlobal pMac, tANI_U32 arg1,
								tANI_U32 arg2, tANI_U32 arg3,
								tANI_U32 arg4, char *p );

char *dump_force_hal_nwtype24g( tpAniSirGlobal pMac, tANI_U32 arg1,
									   tANI_U32 arg2, tANI_U32 arg3,
									   tANI_U32 arg4, char *p );

char *dump_hal_sta_rate_20_set( tpAniSirGlobal pMac, tANI_U32 arg1,
									 tANI_U32 arg2, tANI_U32 arg3,
					 				 tANI_U32 arg4, char *p );

char *dump_hal_sta_rate_40_set( tpAniSirGlobal pMac, tANI_U32 arg1,
				    				 tANI_U32 arg2, tANI_U32 arg3,
								   	 tANI_U32 arg4, char *p );

char *dump_hal_sta_rate_protrate_20_set( tpAniSirGlobal pMac, tANI_U32 arg1,
											   tANI_U32 arg2, tANI_U32 arg3,
											   tANI_U32 arg4, char *p );

char *dump_hal_sta_rate_protrate_40_set( tpAniSirGlobal pMac, tANI_U32 arg1,
											   tANI_U32 arg2, tANI_U32 arg3,
											   tANI_U32 arg4, char *p );

char *dump_hal_rate_set_txPower( tpAniSirGlobal pMac, tANI_U32 arg1,
									   tANI_U32 arg2, tANI_U32 arg3,
									   tANI_U32 arg4, char *p );

char *dump_hal_set_global_enable_rates( tpAniSirGlobal pMac, tANI_U32 arg1,
						   					   tANI_U32 arg2, tANI_U32 arg3,
											   tANI_U32 arg4, char *p );

char *dump_hal_set_global_rates_maxTput( tpAniSirGlobal pMac, tANI_U32 arg1,
											     tANI_U32 arg2, tANI_U32 arg3,
											     tANI_U32 arg4, char *p );

char *dump_phy_rx_counters( tpAniSirGlobal pMac, tANI_U32 arg1,
								  tANI_U32 arg2, tANI_U32 arg3,
								  tANI_U32 arg4, char *p );

char *dump_hal_enable_radar_interrupt( tpAniSirGlobal pMac, tANI_U32 arg1,
											 tANI_U32 arg2, tANI_U32 arg3,
											 tANI_U32 arg4, char *p );


char *dump_hal_tpe_transmitted_pktcount_by_rate ( tpAniSirGlobal pMac, tANI_U32 arg1,
	                                                      tANI_U32 arg2, tANI_U32 arg3,
	                                                      tANI_U32 arg4, char *p );

char *dump_hal_tpe_ratetable_send( tpAniSirGlobal pMac, tANI_U32 arg1,
	                                     tANI_U32 arg2, tANI_U32 arg3,
	                                     tANI_U32 arg4, char *p );

char *dump_hal_tpe_rspratetable_send( tpAniSirGlobal pMac, tANI_U32 arg1,
	                                         tANI_U32 arg2, tANI_U32 arg3,
	                                         tANI_U32 arg4, char *p );

char *dump_hal_tpe_ratetable( tpAniSirGlobal pMac, tANI_U32 arg1,
	                               tANI_U32 arg2, tANI_U32 arg3,
	                               tANI_U32 arg4, char *p );

char *dump_hal_tpe_rspratetable( tpAniSirGlobal pMac, tANI_U32 arg1,
	                                  tANI_U32 arg2, tANI_U32 arg3,
	                                  tANI_U32 arg4, char *p );

char *dump_hal_tpe_rsprate_change( tpAniSirGlobal pMac, tANI_U32 arg1,
	                                      tANI_U32 arg2, tANI_U32 arg3,
	                                      tANI_U32 arg4, char *p );

char *dump_hal_tpe_ratetable_subband( tpAniSirGlobal pMac, tANI_U32 arg1,
											  tANI_U32 arg2, tANI_U32 arg3,
											  tANI_U32 arg4, char *p );

void halIbssRelatedRegisterDump(tpAniSirGlobal pMac);

#endif

