/*********************************************************************
*                      Hangzhou Lingzhi Lzlinks                      *
*                        Internet of Things                          *
**********************************************************************
*                                                                    *
*            (c) 2018 - 8102 Hangzhou Lingzhi Lzlinks                *
*                                                                    *
*       www.lzlinks.com     Support: embedzjh@gmail.com              *
*                                                                    *
**********************************************************************
*                                                                    *
*       lz_bitops.h *                                                *
*                                                                    *
**********************************************************************
*/
#ifndef __LZ_BITOPS_H__
#define __LZ_BITOPS_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef LZ_BIT_PER_LONG
#   define LZ_BIT_PER_LONG  32
#endif

#define LZ_BITS_PER_BYTE    8

/** \brief bitsת��Ϊ�ֽڸ��� */
#define LZ_BIT_BYTE(n)              ((n) >> 3)

/** \brief ����n bits���� */
#define LZ_BIT_MASK(n)              (1UL << ((n) % LZ_BIT_PER_LONG))

/** \brief bit��λ */
#define LZ_BIT(bit)                 (1u << (bit))

/** \brief bit��λ */
#define LZ_BIT_SET(data, bit)       ((data) |= LZ_BIT(bit))

/** \brief bit���� */
#define LZ_BIT_CLR(data, bit)       ((data) &= ~LZ_BIT(bit))

/** \brief bit��ת */
#define LZ_BIT_TOGGLE(data, bit)    ((data) ^= LZ_BIT(bit))

/** \brief ����bit�Ƿ���λ */
#define LZ_BIT_ISSET(data, bit)     ((data) & LZ_BIT(bit))

/** \brief ��ȡbitֵ */
#define LZ_BIT_GET(data, bit)       (LZ_BIT_ISSET(data, bit) ? 1 : 0)

/** \brief bit��λ, ���� mask ָ����λ */
#define LZ_BIT_SET_MASK(data, mask)         ((data) |= (mask))

/** \brief bit����, ���� mask ָ����λ */
#define LZ_BIT_CLR_MASK(data, mask)         ((data) &= ~(mask))

/** \brief ��ȡ n bits ����ֵ */
#define LZ_BITS_MASK(n)             (~((~0u) << (n)))

/** \brief ��ȡλ��ֵ */
#define LZ_BITS_GET(data, start, len)   \
                   (((data) >> (start)) & LZ_BITS_MASK(len))

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
