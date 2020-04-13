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

/** \brief bits转换为字节个数 */
#define LZ_BIT_BYTE(n)              ((n) >> 3)

/** \brief 计算n bits掩码 */
#define LZ_BIT_MASK(n)              (1UL << ((n) % LZ_BIT_PER_LONG))

/** \brief bit移位 */
#define LZ_BIT(bit)                 (1u << (bit))

/** \brief bit置位 */
#define LZ_BIT_SET(data, bit)       ((data) |= LZ_BIT(bit))

/** \brief bit清零 */
#define LZ_BIT_CLR(data, bit)       ((data) &= ~LZ_BIT(bit))

/** \brief bit翻转 */
#define LZ_BIT_TOGGLE(data, bit)    ((data) ^= LZ_BIT(bit))

/** \brief 测试bit是否置位 */
#define LZ_BIT_ISSET(data, bit)     ((data) & LZ_BIT(bit))

/** \brief 获取bit值 */
#define LZ_BIT_GET(data, bit)       (LZ_BIT_ISSET(data, bit) ? 1 : 0)

/** \brief bit置位, 根据 mask 指定的位 */
#define LZ_BIT_SET_MASK(data, mask)         ((data) |= (mask))

/** \brief bit清零, 根据 mask 指定的位 */
#define LZ_BIT_CLR_MASK(data, mask)         ((data) &= ~(mask))

/** \brief 获取 n bits 掩码值 */
#define LZ_BITS_MASK(n)             (~((~0u) << (n)))

/** \brief 获取位段值 */
#define LZ_BITS_GET(data, start, len)   \
                   (((data) >> (start)) & LZ_BITS_MASK(len))

#ifdef __cplusplus
}
#endif

#endif
/*************************** End of file ****************************/
