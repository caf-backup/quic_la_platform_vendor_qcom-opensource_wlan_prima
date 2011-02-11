/* 
 * Airgo Networks, Inc proprietary. All rights reserved
 * L3 IPv4 classifier to support WSM
 * Author:  Pierre Vandwalle	
 * Date:	08/21/2004
 * 
 * History:-
 * Date		Modified by			Modification Information
 * --------------------------------------------------------------------------
 *
 */
#ifndef __SYS_L3_CLASSIFIER_H
#define __SYS_L3_CLASSIFIER_H

#include "aniGlobal.h"
#include "sirApi.h"

#define ETH_ADDR_LEN		6

#define IP_TYPE			0x800

#define IP_VERSION_4		0x4

#define SRC_IP_MASK		0x02
#define SRC_PORT_MASK		0x08
#define DST_PORT_MASK		0x10
#define DSCP_MASK		0x20
#define PROTO_MASK		0x40

#define IP_TOS_MASK		0x3F

typedef struct sethh {
	tANI_U8			dstAddr[ETH_ADDR_LEN];
	tANI_U8			srcAddr[ETH_ADDR_LEN];
	tANI_U16			type;
} tethh;

typedef struct siph {
#ifdef ANI_LITTLE_BIT_ENDIAN
	tANI_U8			length:4,
				version:4;
#else
	tANI_U8			version:4,
  				length:4;
#endif
	tANI_U8			tos;
	tANI_U16			total_len;
	tANI_U16			id;
	tANI_U16			frag_offset;
	tANI_U8			ttl;
	tANI_U8			protocol;
	tANI_U16			checksum;
	tANI_U32			ips;
	tANI_U32			ipd;
} tiph;

typedef struct sudph {
	tANI_U16			ps;
	tANI_U16			pd;
	tANI_U16			length;
	tANI_U16			checksum;
} tudph;

typedef struct sClassifierList {
	tSirTclasInfo		clasInfo;
	tANI_U8			clsId;
	tANI_U8			tid;
	tANI_U32			hits;
	struct sClassifierList	*next;
} tClassifierList;

/*
 * Five API calls are needed for the classifier function:
 * Init Function :
 * 	Initialization function
 * Control Path:
 *    addClassifier: adds the specified classifier
 *    delClassifier: removes the specified classifier
 *    delStaClassifiers: removes all classifiers associated with a STA
 * Data Path:
 *    classifyPacket: used in the data path to request a TID for a packet
 * 
 * LIM will call the control functions when the associated TSPEC signalling
 * happens (add and delete classifiers). When a STA disassociates, all classifiers
 * for that STA are deleted (by LIM).
 * 
 * For each data packet, DPH will call the classifyPacket routine if the packet is
 * destined for a STA that:
 *    is WSM associated, and
 *    has negotiated at least one TSPEC with classifiers
 *
 * Would be nice to have a debugging hook to dump out all classifiers in the
 * the table, or all classifiers related to a specific STA
 */

/* 
 * initialize function
 * return values
 *      OK: initialized successfully
 */
tSirRetStatus initClassifier(tpAniSirGlobal pMac);

/* 
 * add the specified classifier for the specified sta
 * the tid is the value to be returned on successful packet classification match
 * return values
 *      OK: added calssifier successfully
 *      DUP: specified classifier is already present
 *      other errors such as invliad params, unsupported classifier, etc.
 */
extern tSirRetStatus addClassifier(tpAniSirGlobal pMac, tANI_U16 staid, 
					tSirTclasInfo *pTclas, tANI_U8 clsId, tANI_U8 tid); 

/* 
 * delete the specified classifier for the specified sta
 * param
 * 	clsId - 4bits of tsid & 2bits of direction
 * return values
 *      OK: success
 *      other errors such as invalid params, unsupported classifier, etc.
 */
extern tSirRetStatus delClassifier(tpAniSirGlobal pMac, tANI_U16 staid, tANI_U8 clsId);

/* 
 * delete all classifiers associated with the specified sta
 * return values
 *      OK: success or no classifiers for the sta
 *      other errors
 */
extern tSirRetStatus delStaClassifiers(tpAniSirGlobal pMac, tANI_U16 staid); 

/* 
 * find a match for the given data packet and return the TID for the match
 * param values
 * 	pSkb - pointer to Skb
 * 	tid - on successful match, tid for the packet is filled in
 * return values
 *      OK: found a match, *tid contains the assocaited TID
 *      error: no match found
 */
tSirRetStatus classifyPacket(tpAniSirGlobal pMac, tANI_U16 staid, 
		void *pSkb[], tANI_U32 numEntries, tANI_U8 *tid);

// Used for debugging
char *newl3test(tpAniSirGlobal pMac, char *p, tANI_U32 arg1, 
			tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4);

#endif /* __SYS_L3_CLASSIFIER_H */
