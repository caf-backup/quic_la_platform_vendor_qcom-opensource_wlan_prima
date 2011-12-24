/* 
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * L3 IPv4 classifier
 * Author:  Pierre Vandwalle	
 * Date:	08/14/2003
 * 
 * History:-
 * Date		Modified by			Modification Information
 * --------------------------------------------------------------------------
 *
 */


#define TCP_PROTO 6
#define UDP_PROTO 17

struct iph {
#ifdef ANI_LITTLE_BIT_ENDIAN
	tANI_U8	length:4,
		version:4;
#else
	tANI_U8	version:4,
  		length:4;
#endif
    tANI_U8   tos;
	tANI_U16	 total_len;
	tANI_U16	id;
	tANI_U16 frag_offset;
	tANI_U8	ttl;
	tANI_U8	protocol;
	tANI_U16	checksum;
	tANI_U32	ips;
	tANI_U32	ipd;
};

struct udph {
	tANI_U16	ps;
	tANI_U16	pd;
	tANI_U16	length;
	tANI_U16	checksum;
};


struct hentry {
	tANI_U32 control; // 16 bits for now: class_id&0x7f | (proto<<1)<<7 | (tos|0xff)<<8 
	tANI_U32 ips; //ip source
	tANI_U32 ipd; //ip dest
	tANI_U32  p; //port source | port dest
	struct hentry * next; //hash double linked list
	struct hentry * prev;
	struct hentry * mgmt_next; //management list: free list , per classifier list 
	tANI_U32 hits; //number of hits
	
};

struct classifier {
	tANI_U32 ips_mask;
	tANI_U32 ipd_mask;
	tANI_U32 p_mask;
	tANI_U32 class_mask; // mask the (tos|proto|classifier) half word
	tANI_U8 class_id; //7 bits
	tANI_U8 priority; // order in which classifier are evaluated
	tANI_U16 res;
	struct classifier * next; //free single linked list or per port doule linked list
	struct classifier * prev;
	struct hentry * list; //list of entries for this classifier

};


extern void init_hentry(void);
extern void init_hclass(void);
extern struct hentry * get_tid(struct iph * iph, struct classifier * classifier,tANI_U16 * hbuckets,struct hentry *harray);
extern void insert_classifier(struct classifier * classifier, struct classifier ** list);
extern struct classifier * allocate_new_classifier(void);
extern void free_classifier(struct classifier * classifier);
