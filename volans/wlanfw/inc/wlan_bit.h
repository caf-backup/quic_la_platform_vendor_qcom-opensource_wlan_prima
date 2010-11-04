#ifndef BIT_H
#define BIT_H

#ifdef __cplusplus
extern "C" 
{
#endif

#define MAKE_MASK(numBits) ((0x1L << numBits) - 1)
#define TWOS_COMP(value, numBits)  ((~(value) & MAKE_MASK(numBits)) + 1)

#define GET_MAG(sval) (sval < 0 ? -sval : sval)
#define GET_ROUND(val1, val2) ((((((val1) % (val2)) * 10) / (val2)) < 5) ? 0 : 1)
#define GET_MIN(val1, val2) ((val1 < val2) ? val1 : val2)



//READ_MASKED_BITS parses out the requested bits from the register value
#define READ_MASKED_BITS(regValue, mask, offset) \
    ((regValue & (mask << offset)) >> offset)





#define BIT_0   0x00000001
#define BIT_1   0x00000002
#define BIT_2   0x00000004
#define BIT_3   0x00000008
#define BIT_4   0x00000010
#define BIT_5   0x00000020
#define BIT_6   0x00000040
#define BIT_7   0x00000080
#define BIT_8   0x00000100
#define BIT_9   0x00000200
#define BIT_10  0x00000400
#define BIT_11  0x00000800
#define BIT_12  0x00001000
#define BIT_13  0x00002000
#define BIT_14  0x00004000
#define BIT_15  0x00008000
#define BIT_16  0x00010000
#define BIT_17  0x00020000
#define BIT_18  0x00040000
#define BIT_19  0x00080000
#define BIT_20  0x00100000
#define BIT_21  0x00200000
#define BIT_22  0x00400000
#define BIT_23  0x00800000
#define BIT_24  0x01000000
#define BIT_25  0x02000000
#define BIT_26  0x04000000
#define BIT_27  0x08000000
#define BIT_28  0x10000000
#define BIT_29  0x20000000
#define BIT_30  0x40000000
#define BIT_31  0x80000000

#define MSK_1   0x1
#define MSK_2   0x3
#define MSK_3   0x7
#define MSK_4   0xF
#define MSK_5   0x1F
#define MSK_6   0x3F
#define MSK_7   0x7F
#define MSK_8   0xFF
#define MSK_9   0x1FF
#define MSK_10  0x3FF
#define MSK_11  0x7FF
#define MSK_12  0xFFF
#define MSK_13  0x1FFF
#define MSK_14  0x3FFF
#define MSK_15  0x7FFF
#define MSK_16  0xFFFF
#define MSK_17  0x1FFFF
#define MSK_18  0x3FFFF
#define MSK_19  0x7FFFF
#define MSK_20  0xFFFFF
#define MSK_21  0x1FFFFF
#define MSK_22  0x3FFFFF
#define MSK_23  0x7FFFFF
#define MSK_24  0xFFFFFF
#define MSK_25  0x1FFFFFF
#define MSK_26  0x3FFFFFF
#define MSK_27  0x7FFFFFF
#define MSK_28  0xFFFFFFF
#define MSK_29  0x1FFFFFFF
#define MSK_30  0x3FFFFFFF
#define MSK_31  0x7FFFFFFF
#define MSK_32  0xFFFFFFFF


#ifdef __cplusplus
}
#endif

#endif
