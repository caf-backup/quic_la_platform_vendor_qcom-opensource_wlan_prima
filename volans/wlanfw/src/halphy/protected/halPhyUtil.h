#if !defined( __HAL_PHY_UTIL_H__ )
#define __HAL_PHY_UTIL_H__ 

#if defined(BYTE_ORDER_LITTLE_ENDIAN) && !defined(BYTE_ORDER_BIG_ENDIAN)

#define _HTONS(A) (A)
#define _HTONL(A) (A)



#elif defined(BYTE_ORDER_BIG_ENDIAN) && !defined(BYTE_ORDER_LITTLE_ENDIAN)

#define _HTONS(A)  ((((tANI_U16)(A) & 0xff00) >> 8) | \
                    (((tANI_U16)(A) & 0x00ff) << 8)   \
                   )
#define _HTONL(A)  ((((tANI_U32)(A) & 0xff000000) >> 24) | \
                    (((tANI_U32)(A) & 0x00ff0000) >> 8)  | \
                    (((tANI_U32)(A) & 0x0000ff00) << 8)  | \
                    (((tANI_U32)(A) & 0x000000ff) << 24)   \
                   )


#else

#error "Either BYTE_ORDER_BIG_ENDIAN or BYTE_ORDER_LITTLE_ENDIAN must be #defined, but not both."

#endif

#ifdef HTONS
#undef HTONS
#endif

#ifdef HTONL
#undef HTONL
#endif

#ifdef NTOHS
#undef NTOHS
#endif

#ifdef NTOHL
#undef NTOHL
#endif

#define HTONS(A) { A = _HTONS(A); }
#define HTONL(A) { A = _HTONL(A); }

#define NTOHS(A) { A = _HTONS(A); }
#define NTOHL(A) { A = _HTONL(A); }

#define BYTE_SWAP_L(A) { HTONL(A); }
#define BYTE_SWAP_S(A) { HTONS(A); }
#endif
