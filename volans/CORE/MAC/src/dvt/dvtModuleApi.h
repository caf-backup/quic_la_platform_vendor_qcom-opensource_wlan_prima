#ifndef DVTMODULEAPI_H
#define DVTMODULEAPI_H

#include "dvtMsgApi.h"
#include "aniGlobal.h"

eANI_DVT_STATUS dvtInitAll(tpAniSirGlobal pMac);
eANI_DVT_STATUS dvtWriteReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value);
eANI_DVT_STATUS dvtReadReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value);
eANI_DVT_STATUS dvtWriteMemory(tpAniSirGlobal pMac, tANI_U32 macDestAddr, tANI_U8 *pBuf, tANI_U32 bufSize);
eANI_DVT_STATUS dvtReadMemory(tpAniSirGlobal pMac, tANI_U32 macSourceAddr, tANI_U8 *pBuf, tANI_U32 bufSize);
eANI_DVT_STATUS dvtConfigPipe(tpAniSirGlobal pMac, ePipes pipe, sDvtPipeCfg *pDvtPipeCfg);
eANI_DVT_STATUS dvtWritePipeFrameTransfer(tpAniSirGlobal pMac, tDvtSendPacket *pPacketArray, tANI_U32 countPackets);
eANI_DVT_STATUS dvtCreateFrame(tpAniSirGlobal pMac, sDvtFrameSpec spec);
eANI_DVT_STATUS dvtNviWriteData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U8 *pBuf, tANI_U32 nBytes);
eANI_DVT_STATUS dvtNviReadData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U8 *pBuf, tANI_U32 nBytes);
eANI_DVT_STATUS dvtNviWriteBurstData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U32 *pBuf, tANI_U32 nDwords);
eANI_DVT_STATUS dvtNviReadBurstData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U32 *pBuf, tANI_U32 nDwords);
eANI_DVT_STATUS dvtGetEepromFieldSize(tpAniSirGlobal pMac, eEepromField field, tANI_U32 *fieldSize);
eANI_DVT_STATUS dvtGetEepromTableSize(tpAniSirGlobal pMac, eEepromTable table, tANI_U32 *tableSize);
eANI_DVT_STATUS dvtGetEepromTableDir(tpAniSirGlobal pMac, eEepromTable table, sEepromTableDir *dirEntry);
eANI_DVT_STATUS dvtReadEepromField(tpAniSirGlobal pMac, eEepromField field, uEepromFields *fieldData);
eANI_DVT_STATUS dvtWriteEepromField(tpAniSirGlobal pMac, eEepromField field, uEepromFields *fieldData);
eANI_DVT_STATUS dvtReadEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData, tANI_U32 *tableLen);
eANI_DVT_STATUS dvtWriteEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData);
eANI_DVT_STATUS dvtRemoveEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable);

eANI_DVT_STATUS dvtSimpleMacConfig(tpAniSirGlobal pMac, sDvtSimpleMacConfig mac);
eANI_DVT_STATUS dvtSimpleBssConfig(tpAniSirGlobal pMac, sDvtSimpleBssConfig bss, tANI_U32 *bssIndex);
eANI_DVT_STATUS dvtAddSimpleStation(tpAniSirGlobal pMac, sDvtSimpleStationConfig sta, tANI_U32 *stationIndex);
eANI_DVT_STATUS dvtDeleteStation(tpAniSirGlobal pMac, tANI_U32 stationIndex);

eANI_DVT_STATUS dvtAllocTxRxHeaps(tpAniSirGlobal pMac, tANI_U32 txQueueHeap, tANI_U32 rxQueueHeap);
eANI_DVT_STATUS dvtAllocHeap(tpAniSirGlobal pMac, sDvtHeapResources *heap, tANI_U32 numBytes);
eANI_DVT_STATUS dvtFreeHeap(tpAniSirGlobal pMac, sDvtHeapResources *heap);

eANI_DVT_STATUS dvtGetStationTable(tpAniSirGlobal pMac, tDvtMsgGetStaTable *pTable);
eANI_DVT_STATUS dvtMemTest(tpAniSirGlobal pMac, tDvtMemTest memTest, tANI_U32 *failedAddr);

eANI_DVT_STATUS dvtGetDpuSignature(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U8 *dpuSignature);

eANI_DVT_STATUS dvtSetStaKeyParams(tpAniSirGlobal pMac, tSetStaKeyParams keyParams);
eANI_DVT_STATUS dvtSetBssKeyParams(tpAniSirGlobal pMac, tSetBssKeyParams keyParams);

eANI_DVT_STATUS dvtAddStation(tpAniSirGlobal pMac, tAddStaParams sta);
eANI_DVT_STATUS dvtBssConfig(tpAniSirGlobal pMac, tAddBssParams bss);

eANI_DVT_STATUS dvtGatherInfo(tpAniSirGlobal pMac, sDvtGatherInfo *dvtInfo, tANI_U32 dbgInfoMask);
eANI_DVT_STATUS dvtCrcCheckEnable(tpAniSirGlobal pMac, tANI_U32 crcEnable);
eANI_DVT_STATUS dvtDxeTimestampEnable(tpAniSirGlobal pMac, tANI_U32 dxeTimestampEnable);
eANI_DVT_STATUS dvtResetCounters(tpAniSirGlobal pMac);
eANI_DVT_STATUS dvtDumpBdPdu(tpAniSirGlobal pMac, tANI_U32 dumpMasknIndex, tANI_U8 *pBuf);

eANI_DVT_STATUS dvtGetTxCountPerRate(tpAniSirGlobal pMac, tANI_U32 *cTxCountPerRate, tANI_U32 size);
eANI_DVT_STATUS dvtGetRxCountPerRate(tpAniSirGlobal pMac, tANI_U32 *cRxCountPerRate, tANI_U32 size);
eANI_DVT_STATUS dvtGetRxCounters(tpAniSirGlobal pMac, tDvtRxCounters *rxCounters, tANI_U8 size);
eANI_DVT_STATUS dvtGetRxPMIRateIdxMisMatchCount(tpAniSirGlobal pMac, tANI_U32 *cRxPMIRateIdxMisMatchCount, tANI_U32 size);
eANI_DVT_STATUS dvtGetSmacRuntimeStats(tpAniSirGlobal pMac, tDvtSmacRuntimeStat *smacStats, tANI_U8 size);

eANI_DVT_STATUS dvtReadRegs(tpAniSirGlobal pMac, sDvtReadRegs *regs, tANI_U8 size);
eANI_DVT_STATUS dvtGetCountPerSta(tpAniSirGlobal pMac, tANI_U32 *countPerSta, tANI_BOOLEAN isTx);
eANI_DVT_STATUS dvtWriteBeaconToMemory(tpAniSirGlobal pMac, tANI_U8 *beacon, tANI_U16 bssIndex, tANI_U32 length, tpPESession psessionEntry);

eANI_DVT_STATUS dvtSetEepromBurstPageSize(tpAniSirGlobal pMac, tANI_U32 pageSize);
void dvtFrameBoundsInit(tpAniSirGlobal pMac);
eANI_DVT_STATUS dvtAbortFrameSpec(tpAniSirGlobal pMac, eDvtTxQueue queue);
eANI_DVT_STATUS dvtContinueFrameSpec(tpAniSirGlobal pMac, sDvtFrameSpec *spec, tANI_U32 *framesCreated);
void dvtFrameGenCallback(tHalHandle hHal, eDvtTxQueue queue);
void dvtFrameReceivedCallback(tHalHandle hHal, eDvtRxQueue queue, sRxFrameTransfer *rxFrame);


eANI_DVT_STATUS dvtQueueInit(tpAniSirGlobal pMac);
eANI_DVT_STATUS dvtTxEnqueueFrame(tpAniSirGlobal pMac, eDvtTxQueue queue, sTxFrameTransfer *frame);
eHalStatus dvtTxDequeueFrame(tHalHandle hHal, ePipes pipe, uFrameTransfer *frame);
eHalStatus dvtRxEnqueueFrame(tHalHandle hHal, ePipes pipe, uFrameTransfer *frame);
eANI_DVT_STATUS dvtRxDequeueFrame(tpAniSirGlobal pMac, eDvtRxQueue queue, sRxFrameTransfer *frame);
eANI_DVT_STATUS dvtRxXfrFrames(tpAniSirGlobal pMac);
void dvtAppRxEnqueueCallback(tHalHandle hHal, eDvtRxQueue queue, sRxFrameTransfer *rxFrame);

//other message functions
extern void mbReceiveMBMsg( void *mpAdapter, tSirMbMsg *pMsg );
tSirRetStatus dvtReceiveMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);
eANI_DVT_STATUS dvtProcessMsg(tpAniSirGlobal pMac, tDvtMsgbuffer *pDvtMsg);
tSirRetStatus dvtHalReceiveMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg, tDvtMsgbuffer *hDvtMsg);
void dvtSendMsgResponse(tpAniSirGlobal pMac, tDvtMsgbuffer *pDvtMsg);

void dvtDumpDxePktTimeHistogram(tpAniSirGlobal pMac);

void testDvtCreateFrames(tpAniSirGlobal pMac, tANI_U32 threadCount, tANI_U32 totalCount);
void testForceDvtHeapGarbageCleanup(tpAniSirGlobal pMac);
sHddMemSegment *GetHeapBytes(tpAniSirGlobal pMac, sDvtHeapResources *heap, tANI_U32 numBytes, eDvtHeapAccess access);
void PutHeapBytes(tpAniSirGlobal pMac, sDvtHeapResources *heap, sHddMemSegment *garbageList);
void dvtDumpInit(tHalHandle hHal);


#endif
