#ifndef HAL_BD_DEFS_H
#define HAL_BD_DEFS_H


#ifndef  __ASSEMBLER__

#define QWLAN_RXBD_DEFRAG_BIT 0x1

typedef struct sHalRxBd {
        /* 0x00 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** (Only used by the DPU)
        This routing flag indicates the WQ number to which the DPU will push the
        frame after it finished processing it. */
        tANI_U32 dpuRF:8;
    
        /** This is DPU sig inserted by RXP. Signature on RA's DPU descriptor */
        tANI_U32 dpuSignature:3;
    
        /** When set Sta is authenticated. SW needs to set bit
        addr2_auth_extract_enable in rxp_config2 register. Then RXP will use bit 3
        in DPU sig to say whether STA is authenticated or not. In this case only
        lower 2bits of DPU Sig is valid */
        tANI_U32 stAuF:1;
    
        /** When set address2 is not valid??? */
        tANI_U32 A2HF:1;
    
        /** When set it indicates TPE has sent the Beacon frame */
        tANI_U32 bsf:1;
    
        /** This bit filled by rxp when set indicates if the current tsf is smaller
        than received tsf */
        tANI_U32 rtsf:1;
    
        /** These two fields are used by SW to carry the Rx Channel number and SCAN bit in RxBD*/
        tANI_U32 rxChannel:4;
        tANI_U32 scanLearn:1;

#if defined(LIBRA_WAPI_SUPPORT)
        /** UnEncrypted Frame received over WAPI channel, only for WAPI*/
        tANI_U32 uef:1;   
#else
        tANI_U32 reserved0:1;
#endif
    
        /** LLC Removed
        This bit is only used in Libra rsvd for Virgo1.0/Virgo2.0
        Filled by ADU when it is set LLC is removed from packet */
        tANI_U32 llcr:1;
        
        tANI_U32 umaByPass:1;
    
        /** This bit is only available in Virgo2.0/libra it is reserved in Virgo1.0
        Robust Management frame. This bit indicates to DPU that the packet is a
        robust management frame which requires decryption(this bit is only valid for
        management unicast encrypted frames)
        1 - Needs decryption
        0 - No decryption required */
        tANI_U32 rmf:1;
    
        /** 
        This bit is only in Virgo2.0/libra it is reserved in Virgo 1.0
        This 1-bit field indicates to DPU Unicast/BC/MC packet
        0 - Unicast packet
        1 - Broadcast/Multicast packet
        This bit is only valid when RMF bit is 1 */
        tANI_U32 ub:1;
    
        /** This is the KEY ID extracted from WEP packets and is used for determine
        the RX Key Index to use in the DPU Descriptror.
        This field  is 2bits for virgo 1.0
        And 3 bits in virgo2.0 and Libra
        In virgo2.0/libra it is 3bits for the BC/MC packets */
        tANI_U32 rxKeyId:3;
        
        /**  (Only used by the DPU)    
        No encryption/decryption
        0: No action
        1: DPU will not encrypt/decrypt the frame, and discard any encryption
        related settings in the PDU descriptor. */
        tANI_U32 dpuNE:1;
    
        /** 
        This is only available in libra/virgo2.0  it is reserved for virgo1.0
        This bit is filled by RXP and modified by ADU
        This bit indicates to ADU/UMA module that the packet requires 802.11n to
        802.3 frame translation. Once ADU/UMA is done with translation they
        overwrite it with 1'b0/1'b1 depending on how the translation resulted
        When used by ADU 
        0 - No frame translation required
        1 - Frame Translation required
        When used by SW
        0 - Frame translation not done, MPDU header offset points to 802.11 header..
        1 - Frame translation done ;  hence MPDU header offset will point to a
        802.3 header */
        tANI_U32 ft:1;
    
        /** (Only used by the DPU)
        BD Type 
        00: 'Generic BD', as indicted above
        01: De-fragmentation format 
        10-11: Reserved for future use. */
        tANI_U32 bdt:2;
        
#else
        tANI_U32 bdt:2;
        tANI_U32 ft:1;
        tANI_U32 dpuNE:1;
        tANI_U32 rxKeyId:3;
        tANI_U32 ub:1;
        tANI_U32 rmf:1;
        tANI_U32 reserved1:1;
        tANI_U32 llc:1;
#if defined(LIBRA_WAPI_SUPPORT)
        /** UnEncrypted Frame received over WAPI channel, only for WAPI*/
        tANI_U32 uef:1;   
#else
        tANI_U32 reserved0:1;
#endif
        tANI_U32 scanLearn:1;
        tANI_U32 rxChannel:4;
        tANI_U32 rtsf:1;
        tANI_U32 bsf:1;
        tANI_U32 A2HF:1;
        tANI_U32 stAuF:1;
        tANI_U32 dpuSignature:3;
        tANI_U32 dpuRF:8;
#endif
    
        /* 0x04 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** This is used for AMSDU this is the PDU index of the PDU which is the
        one before last PDU; for all non AMSDU frames, this field SHALL be 0.
        Used in ADU (for AMSDU deaggregation) */
        tANI_U32 penultimatePduIdx:16;
    
        tANI_U32 aduFeedback:8;
    
        /** DPU feedback */
        tANI_U32 dpuFeedback:8;
        
#else
        tANI_U32 dpuFeedback:8;
        tANI_U32 aduFeedback:8;
        tANI_U32 penultimatePduIdx:16;
#endif
    
        /* 0x08 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** In case PDUs are linked to the BD, this field indicates the index of
        the first PDU linked to the BD. When PDU count is zero, this field has an
        undefined value. */
        tANI_U32 headPduIdx:16;
    
        /** In case PDUs are linked to the BD, this field indicates the index of
        the last PDU. When PDU count is zero, this field has an undefined value.*/
        tANI_U32 tailPduIdx:16;
        
#else
        tANI_U32 tailPduIdx:16;
        tANI_U32 headPduIdx:16;
#endif
    
        /* 0x0c */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** The length (in number of bytes) of the MPDU header. 
        Limitation: The MPDU header offset + MPDU header length can never go beyond
        the end of the first PDU */
        tANI_U32 mpduHeaderLength:8;
    
        /** The start byte number of the MPDU header. 
        The byte numbering is done in the BE format. Word 0x0, bits [31:24] has
        byte index 0. */
        tANI_U32 mpduHeaderOffset:8;
    
        /** The start byte number of the MPDU data. 
        The byte numbering is done in the BE format. Word 0x0, bits [31:24] has
        byte index 0. Note that this offset can point all the way into the first
        linked PDU.
        Limitation: MPDU DATA OFFSET can not point into the 2nd linked PDU */
        tANI_U32 mpduDataOffset:9;
    
        /** The number of PDUs linked to the BD. 
        This field should always indicate the correct amount. */
        tANI_U32 pduCount:7;
#else
    
        tANI_U32 pduCount:7;
        tANI_U32 mpduDataOffset:9;
        tANI_U32 mpduHeaderOffset:8;
        tANI_U32 mpduHeaderLength:8;
#endif
    
        /* 0x10 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** This is the length (in number of bytes) of the entire MPDU 
        (header and data). Note that the length does not include FCS field. */
        tANI_U32 mpduLength:16;
    
        tANI_U32 reserved3:4;
    
        /** Traffic Identifier
        Indicates the traffic class the frame belongs to. For non QoS frames,
        this field is set to zero. */
        tANI_U32 tid:4;
        
        tANI_U32 reserved4:8;
#else
        tANI_U32 reserved4:8;
        tANI_U32 tid:4;
        tANI_U32 reserved3:4;
        tANI_U32 mpduLength:16;
#endif
    
        /* 0x14 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** (Only used by the DPU)
        The DPU descriptor index is used to calculate where in memory the DPU can
        find the DPU descriptor related to this frame. The DPU calculates the
        address by multiplying this index with the DPU descriptor size and adding
        the DPU descriptors base address. The DPU descriptor contains information
        specifying the encryption and compression type and contains references to
        where encryption keys can be found. */
        tANI_U32 dpuDescIdx:8;
    
        /** The result from the binary address search on the ADDR1 of the incoming
        frame. See chapter: RXP filter for encoding of this field. */
        tANI_U32 addr1Index:8;
    
        /** The result from the binary address search on the ADDR2 of the incoming
        frame. See chapter: RXP filter for encoding of this field. */
        tANI_U32 addr2Index:8;
    
        /** The result from the binary address search on the ADDR3 of the incoming
        frame. See chapter: RXP filter for encoding of this field. */
        tANI_U32 addr3Index:8;
#else
        tANI_U32 addr3Index:8;
        tANI_U32 addr2Index:8;
        tANI_U32 addr1Index:8;
        tANI_U32 dpuDescIdx:8;
#endif
    
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** Indicates Rate Index of packet received ??? */
        tANI_U32 rateIndex:9;
    
        /** An overview of RXP status information related to receiving the frame.*/
        tANI_U32 rxpFlags:23; 
    
#else
    
        tANI_U32 rxpFlags:23;                     /* RxP flags*/
        tANI_U32 rateIndex:9;
    
#endif
        /* 0x1c, 20 */
        /** The PHY can be programmed to put all the PHY STATS received from the
        PHY when receiving a frame in the BD.  */
        tANI_U32 phyStats0;                      /* PHY status word 0*/
        tANI_U32 phyStats1;                      /* PHY status word 1*/
    
        /* 0x24 */
        /** The value of the TSF[31:0] bits at the moment that the RXP start
        receiving a frame from the PHY RX. */
        tANI_U32 mclkRxTimestamp;                /* Rx timestamp, microsecond based*/
    
        /* 0x28~0x38 */
        /** The bits from the PMI command as received from the PHY RX. */
        tANI_U32 pmiCmd4to23[5];               /* PMI cmd rcvd from RxP */
    
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** The bits from the PMI command as received from the PHY RX. */
        tANI_U32 pmiCmd24:8;    
        tANI_U32 pmiCmd25:8;
    
        tANI_U32 reserved5:16;
#else
        tANI_U32 reserved5:16;
        tANI_U32 pmiCmd25:8;
        tANI_U32 pmiCmd24:8;
#endif
    
        /* 0x3c */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** Gives commands to software upon which host will perform some commands.
        Please refer to following RPE document for description of all different
        values for this field. */
        tANI_U32 reorderOpcode:4;
    
        tANI_U32 reserved6:12;
    
        /** Filled by RPE to Indicate to the host up to which slot the host needs
        to forward the packets to upper Mac layer. This field mostly used for AMDPU
        packets */
        tANI_U32 reorderFwdIdx:6;
    
        /** Filled by RPE which indicates to the host which one of slots in the
        available 64 slots should the host Queue the packet. This field only
        applied to AMPDU packets. */
        tANI_U32 reorderSlotIdx:6;
        
        tANI_U32 reserved7:4;
#else
        tANI_U32 reserved7:4;
        tANI_U32 reorderSlotIdx:6;
        tANI_U32 reorderFwdIdx:6;
        tANI_U32 reserved6:12;
        tANI_U32 reorderOpcode:4;
#endif
    
        /* 0x40 */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** reserved8 from a hardware perspective.
        Used by SW to propogate frame type/subtype information */
        tANI_U32 frameTypeSubtype:8;
    
        /** Filled RPE gives the current sequence number in bitmap */
        tANI_U32 currentPktSeqNo:12;
    
        /** Filled by RPE which gives the sequence number of next expected packet
        in bitmap */
        tANI_U32 expectedPktSeqNo:12;
#else
        tANI_U32 expectedPktSeqNo:12;
        tANI_U32 currentPktSeqNo:12;
        tANI_U32 frameTypeSubtype:8;
#endif
    
        /* 0x48 */
#ifdef ANI_BIG_BYTE_ENDIAN
    
        /** When set it is the AMSDU subframe */
        tANI_U32 asf:1;
    
        /** When set it is the First subframe of the AMSDU packet */
        tANI_U32 esf:1;
    
        /** When set it is the last subframe of the AMSDU packet */
        tANI_U32 lsf:1;
    
        /** When set it indicates an Errored AMSDU packet */
        tANI_U32 aef:1;
        
        tANI_U32 reserved9:4;
    
        /** It gives the order in which the AMSDU packet is processed
        Basically this is a number which increments by one for every AMSDU frame
        received. Mainly for debugging purpose. */
        tANI_U32 processOrder:4;
    
        /** It is the order of the subframe of AMSDU that is processed by ADU.
        This is reset to 0 when ADU deaggregates the first subframe from a new
        AMSDU and increments by 1 for every new subframe deaggregated within the
        AMSDU, after it reaches 4'hf it stops incrementing. That means host should
        not rely on this field as index for subframe queuing.  Theoretically there
        can be way more than 16 subframes in an AMSDU. This is only used for debug
        purpose, SW should use LSF and FSF bits to determine first and last
        subframes. */
        tANI_U32 sybFrameIdx:4;
    
        /** Filled by ADU this is the total AMSDU size */
        tANI_U32 totalMsduSize:16;
#else
        tANI_U32 totalMsduSize:16;
        tANI_U32 sybFrameIdx:4;
        tANI_U32 processOrder:4;
        tANI_U32 reserved9:4;
        tANI_U32 aef:1;
        tANI_U32 lsf:1;
        tANI_U32 esf:1;
        tANI_U32 asf:1;
#endif

//} __ani_attr_packed __ani_attr_aligned_4 halRxBd_type, *phalRxBd_type;
} halRxBd_type, *phalRxBd_type;

typedef struct sHmacTxBd {
        /* 0x00 */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** (Only used by the DPU) This routing flag indicates the WQ number to
        which the DPU will push the frame after it finished processing it. */
        tANI_U32 dpuRF:8;
    
        /** DPU signature. Filled by Host in Virgo 1.0 but by ADU in Virgo 2.0 */
        tANI_U32 dpuSignature:3;
    
        tANI_U32 reserved0:12;
    
        /** Only available in Virgo 2.0 and reserved in Virgo 1.0.
        This bit indicates to DPU that the packet is a robust management frame
        which requires  encryption(this bit is only valid for certain management
        frames)
        1 - Needs encryption
        0 - No encrytion required
        It is only set when Privacy bit=1 AND type/subtype=Deauth, Action,
        Disassoc. Otherwise it should always be 0. */
        tANI_U32 rmf:1;
    
        /** This bit is only in Virgo2.0/libra it is reserved in Virgo 1.0
        This 1-bit field indicates to DPU Unicast/BC/MC packet
        0 - Unicast packet
        1 - Broadcast/Multicast packet
        This bit is valid only if RMF bit is set */
        tANI_U32 ub:1;
    
        tANI_U32 reserved1:1;
    
        /**  This bit is only in Virgo2.0/libra it is reserved in Virgo 1.0
        This bit indicates TPE has to assert the TX complete interrupt.
        0 - no interrupt
        1 - generate interrupt */
        tANI_U32 txComplete1:1;
        tANI_U32 fwTxComplete0:1;
        
        /** (Only used by the DPU)
        No encryption/decryption
        0: No action
        1: DPU will not encrypt/decrypt the frame, and discard any encryption
        related settings in the PDU descriptor. */
        tANI_U32 dpuNE:1;
    
        
        /** This is only available in libra/virgo2.0  it is reserved for virgo1.0
        This bit indicates to ADU/UMA module that the packet requires 802.11n
        to 802.3 frame translation. When used by ADU 
        0 - No frame translation required
        1 - Frame Translation required */
        tANI_U32 ft:1;
    
        /** BD Type 
        00: 'Generic BD', as indicted above
        01: De-fragmentation format 
        10-11: Reserved for future use. */
        tANI_U32 bdt:2;
#else
        tANI_U32 bdt:2;
        tANI_U32 ft:1;
        tANI_U32 dpuNE:1;
        tANI_U32 fwTxComplete0:1; 
        tANI_U32 txComplete1:1;
        tANI_U32 reserved1:1;
        tANI_U32 ub:1;
        tANI_U32 rmf:1;
        tANI_U32 reserved0:12;
        tANI_U32 dpuSignature:3;
        tANI_U32 dpuRF:8;
#endif
    
        /* 0x04 */
#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved2:16; /* MUST BE 0 otherwise triggers BMU error*/
        tANI_U32 aduFeedback:8;
    
        /* DPU feedback in Tx path.*/
        tANI_U32 dpuFeedback:8;
    
#else
        tANI_U32 dpuFeedback:8;
        tANI_U32 aduFeedback:8;
        tANI_U32 reserved2:16;
#endif
    
        /* 0x08 */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** It is initially filled by DXE then if encryption is on, then DPU will
        overwrite these fields. In case PDUs are linked to the BD, this field
        indicates the index of the first PDU linked to the BD. When PDU count is
        zero, this field has an undefined value. */
        tANI_U32 headPduIdx:16;
    
        /**  It is initially filled by DXE then if encryption is on, then DPU will
        overwrite these fields.In case PDUs are linked to the BD, this field
        indicates the index of the last PDU. When PDU count is zero, this field
        has an undefined value. */
        tANI_U32 tailPduIdx:16;
#else
        tANI_U32 tailPduIdx:16;
        tANI_U32 headPduIdx:16;
#endif
    
        /* 0x0c */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** This is filled by Host in Virgo 1.0 but it gets changed by ADU in
        Virgo2.0/Libra. The length (in number of bytes) of the MPDU header.
        Limitation: The MPDU header offset + MPDU header length can never go beyond
        the end of the first PDU */
        tANI_U32 mpduHeaderLength:8;
    
        /** This is filled by Host in Virgo 1.0 but it gets changed by ADU in
        Virgo2.0/Libra. The start byte number of the MPDU header. The byte numbering
        is done in the BE format. Word 0x0, bits [31:24] has byte index 0. */
        tANI_U32 mpduHeaderOffset:8;
    
        /** This is filled by Host in Virgo 1.0 but it gets changed by ADU in
        Virgo2.0/Libra. The start byte number of the MPDU data.  The byte numbering
        is done in the BE format. Word 0x0, bits [31:24] has byte index 0.
        Note that this offset can point all the way into the first linked PDU. 
        Limitation: MPDU DATA OFFSET can not point into the 2nd linked PDU */
        tANI_U32 mpduDataOffset:9;
    
        /** It is initially filled by DXE then if encryption is on, then DPU will
        overwrite these fields. The number of PDUs linked to the BD. This field
        should always indicate the correct amount. */
        tANI_U32 pduCount:7;
#else
        tANI_U32 pduCount:7;
        tANI_U32 mpduDataOffset:9;
        tANI_U32 mpduHeaderOffset:8;
        tANI_U32 mpduHeaderLength:8;
#endif
    
        /* 0x10 */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** This is filled by Host in Virgo 1.0 but it gets changed by ADU in
        Virgo2.0/LibraMPDU length.This covers MPDU header length + MPDU data length.
        This does not include FCS. For single frame transmission, PSDU size is
        mpduLength + 4.*/
        tANI_U32 mpduLength:16;
    
        tANI_U32 reserved3:2;
        /** Sequence number insertion by DPU
        00: Leave sequence number as is, as filled by host
        01: DPU to insert non TID based sequence number (If it is not TID based,
        then how does DPU know what seq to fill? Is this the non-Qos/Mgmt sequence
        number?
        10: DPU to insert a sequence number based on TID.
        11: Reserved */
        tANI_U32 bd_ssn:2;
    
        /** Traffic Identifier
        Indicates the traffic class the frame belongs to. For non QoS frames, this
        field is set to zero. */
        tANI_U32 tid:4;
        
        tANI_U32 reserved4:8;
    
#else
        tANI_U32 reserved4:8;
        tANI_U32 tid:4;
        tANI_U32 bd_ssn:2;
        tANI_U32 reserved3:2;
        tANI_U32 mpduLength:16;
#endif
    
        /* 0x14 */
#ifdef ANI_BIG_BYTE_ENDIAN
        /** (Only used by the DPU)
        This is filled by Host in Virgo 1.0 but it gets filled by ADU in
        Virgo2.0/Libra. The DPU descriptor index is used to calculate where in
        memory the DPU can find the DPU descriptor related to this frame. The DPU
        calculates the address by multiplying this index with the DPU descriptor
        size and adding the DPU descriptors base address. The DPU descriptor
        contains information specifying the encryption and compression type and
        contains references to where encryption keys can be found. */
        tANI_U32 dpuDescIdx:8;
    
        /** This is filled by Host in Virgo 1.0 but it gets filled by ADU in
        Virgo2.0/Libra. The STAid of the RA address */
        tANI_U32 staIndex:8;
    
        /** A field passed on to TPE which influences the ACK policy to be used for
        this frame
        00 - Iack
        01,10,11 - No Ack */
        tANI_U32 ap:2;
    
        /** Overwrite option for the transmit rate
        00: Use rate programmed in the TPE STA descriptor
        01: Use TPE BD rate 1
        10: Use TPE BD rate 2
        11: Delayed Use TPE BD rate 3 */
        tANI_U32 bdRate:2;
    
        /** 
        This is filled by Host in Virgo 1.0 but it gets filled by ADU in
        Virgo2.0/Libra. Queue ID */
        tANI_U32 queueId:5;
    
        tANI_U32 reserved5:7;
#else
        tANI_U32 reserved5:7;
        tANI_U32 queueId:5;
        tANI_U32 bdRate:2;
        tANI_U32 ap:2;
        tANI_U32 staIndex:8;
        tANI_U32 dpuDescIdx:8;
#endif

        tANI_U32 txBdSignature;

        /* 0x1C */
        tANI_U32 reserved6;
        /* 0x20 */
        /* Timestamp filled by DXE. Timestamp for current transfer */
        tANI_U32 dxeH2BStartTimestamp;
    
        /* 0x24 */
        /* Timestamp filled by DXE. Timestamp for previous transfer */
        tANI_U32 dxeH2BEndTimestamp;

//} __ani_attr_packed __ani_attr_aligned_4 halTxBd_type, *pHalTxBd_type;
} halTxBd_type, *pHalTxBd_type;

typedef struct sHalRxDeFragBd {
        /* 0x00 */
#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved1:30;
        tANI_U32 bdt:2;
#else
        tANI_U32 bdt:2;
        tANI_U32 reserved1:30;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved2:24;
        tANI_U32 dpuFeedBack:8;
#else
        tANI_U32 dpuFeedBack:8;
        tANI_U32 reserved2:24;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved3:16;
        tANI_U32 frag0BdIdx:16;
#else
        tANI_U32 frag0BdIdx:16;
        tANI_U32 reserved3:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved4:16;
        tANI_U32 frag1BdIdx:16;
#else
        tANI_U32 frag1BdIdx:16;
        tANI_U32 reserved4:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 frag2BdIdx:16;
        tANI_U32 reserved5:16;
#else
        tANI_U32 frag2BdIdx:16;
        tANI_U32 reserved5:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved6:16;
        tANI_U32 frag3BdIdx:16;
#else
        tANI_U32 frag3BdIdx:16;
        tANI_U32 reserved6:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved7:16;
        tANI_U32 frag4BdIdx:16;
#else
        tANI_U32 frag4BdIdx:16;
        tANI_U32 reserved7:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved8:16;
        tANI_U32 frag5BdIdx:16;
#else
        tANI_U32 frag5BdIdx:16;
        tANI_U32 reserved8:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved9:16;
        tANI_U32 frag6BdIdx:16;
#else
        tANI_U32 frag6BdIdx:16;
        tANI_U32 reserved9:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved10:16;
        tANI_U32 frag7BdIdx:16;
#else
        tANI_U32 frag7BdIdx:16;
        tANI_U32 reserved10:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved11:16;
        tANI_U32 frag8BdIdx:16;
#else
        tANI_U32 frag8BdIdx:16;
        tANI_U32 reserved11:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved12:16;
        tANI_U32 frag9BdIdx:16;
#else
        tANI_U32 frag9BdIdx:16;
        tANI_U32 reserved12:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved13:16;
        tANI_U32 frag10BdIdx:16;
#else
        tANI_U32 frag10BdIdx:16;
        tANI_U32 reserved13:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved14:16;
        tANI_U32 frag11BdIdx:16;
#else
        tANI_U32 frag11BdIdx:16;
        tANI_U32 reserved14:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved15:16;
        tANI_U32 frag12BdIdx:16;
#else
        tANI_U32 frag12BdIdx:16;
        tANI_U32 reserved15:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 reserved16:16;
        tANI_U32 frag13BdIdx:16;
#else
        tANI_U32 frag13BdIdx:16;
        tANI_U32 reserved16:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 frag14BdIdx:16;
        tANI_U32 reserved17:16;
#else
        tANI_U32 frag14BdIdx:16;
        tANI_U32 reserved17:16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 frag15BdIdx:16;
        tANI_U32 reserved18:16;
#else
        tANI_U32 frag15BdIdx:16;
        tANI_U32 reserved18:16;
#endif

} halRxDeFragBd_type, *pHalRxDeFragBd_type;

#endif /* __ASSEMBLER__ */

#endif /*HAL_BD_DEFS_H*/
