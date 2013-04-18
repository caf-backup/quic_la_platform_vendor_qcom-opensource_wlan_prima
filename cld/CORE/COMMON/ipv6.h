/*
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
#ifndef _IPV6__H_
#define _IPV6__H_

#if defined(ATH_TARGET)
#include <osapi.h>   /* A_UINT8 */
#else
#include <a_types.h> /* A_UINT8 */
#endif

/* utilities for converting between network byte order and native endianness */
#ifndef BYTESWAP32
#define BYTESWAP32(x) \
    ((((x) & 0x000000ff) << 24) /* byte 0 -> byte 3 */ | \
     (((x) & 0x0000ff00) <<  8) /* byte 1 -> byte 2 */ | \
     (((x) & 0x00ff0000) >>  8) /* byte 2 -> byte 1 */ | \
     (((x) & 0xff000000) >> 24) /* byte 3 -> byte 0 */)
#endif /* BYTESWAP32 */

#ifndef BE_TO_CPU32
    #if defined(ATH_TARGET)
        /* assume target is little-endian */
        #define BE_TO_CPU32(x) BYTESWAP32(x)
    #else
        #ifdef BIG_ENDIAN_HOST
            #define BE_TO_CPU32(x) (x)
        #else
            #define BE_TO_CPU32(x) BYTESWAP32(x)
        #endif
    #endif
#endif /* BE_TO_CPU32 */


/* IPv6 header definition */

#define IPV6_ADDR_LEN 4 /* bytes */
struct ipv6_hdr_t {
    A_UINT32 ver_tclass_flowlabel; /* version, traffic class, and flow label */
    A_UINT8  pyld_len[2];          /* payload length */
    A_UINT8  next_hdr;
    A_UINT8  hop_limit;
    A_UINT8  src_addr[IPV6_ADDR_LEN];
    A_UINT8  dst_addr[IPV6_ADDR_LEN];
};

#define IPV6_HDR_LEN (sizeof(struct ipv6_hdr_t))
#define IPV6_HDR_OFFSET_NEXT_HDR (offsetof(struct ipv6_hdr_t, next_hdr))
#define IPV6_HDR_OFFSET_DST_ADDR (offsetof(struct ipv6_hdr_t, dst_addr[0]))


/* IPv6 header field access macros */

#define IPV6_HDR_VERSION_M       0xF0000000
#define IPV6_HDR_VERSION_S       28

#define IPV6_HDR_TRAFFIC_CLASS_M 0x0FF00000
#define IPV6_HDR_TRAFFIC_CLASS_S 20

#define IPV6_HDR_FLOW_LABEL_M    0x000FFFFF
#define IPV6_HDR_FLOW_LABEL_S    0

static inline A_UINT8 IPV6_VERSION(struct ipv6_hdr_t *ipv6_hdr)
{
    return
        (BE_TO_CPU32(ipv6_hdr->ver_tclass_flowlabel) &
        IPV6_HDR_VERSION_M) >> IPV6_HDR_VERSION_S;
}

static inline A_UINT8 IPV6_TRAFFIC_CLASS(struct ipv6_hdr_t *ipv6_hdr)
{
    return
        (BE_TO_CPU32(ipv6_hdr->ver_tclass_flowlabel) &
        IPV6_HDR_TRAFFIC_CLASS_M) >> IPV6_HDR_TRAFFIC_CLASS_S;
}

static inline A_UINT32 IPV6_FLOW_LABEL(struct ipv6_hdr_t *ipv6_hdr)
{
    return
        (BE_TO_CPU32(ipv6_hdr->ver_tclass_flowlabel) &
        IPV6_HDR_FLOW_LABEL_M) >> IPV6_HDR_FLOW_LABEL_S;
}

#endif /* _IPV6__H_ */
