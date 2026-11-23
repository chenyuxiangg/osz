#ifndef __MEM_FEATURE_H__
#define __MEM_FEATURE_H__

/*
 * mem 模块：配置宏 -> 功能宏的唯一派生口。
 *
 * 规则（见 doc/kernel_doc/mem/内存管理模块详细设计 6.4/6.5）：
 *   1. OSZ_CFG_MEM_* 只允许出现在本文件，业务 .c/.h 一律引用功能宏；
 *   2. 功能宏恒为 0/1，未定义、=0、=1 之外的值直接 #error 编译失败；
 *   3. 块头尺寸、对齐粒度、最小切分阈值是结构常量，不做成配置项，
 *      避免同一份源码在宿主与目标机取到不同值而破坏 ABI/行为一致性。
 *
 * 注意：Kconfig 生成头的语义是"开 = #define X 1，关 = 不定义"，
 * 因此这里必须同时处理"未定义"与"=0"两种情况。
 */

#include "menuconfig.h"

#if defined(OSZ_CFG_MEM_DEBUG) && (OSZ_CFG_MEM_DEBUG == 1)
#define MEM_FEATURE_DEBUG               (1)
#elif defined(OSZ_CFG_MEM_DEBUG) && (OSZ_CFG_MEM_DEBUG == 0)
#define MEM_FEATURE_DEBUG               (0)
#elif !defined(OSZ_CFG_MEM_DEBUG)
#define MEM_FEATURE_DEBUG               (0)
#else
#error "OSZ_CFG_MEM_DEBUG must be 0/1 or undefined"
#endif

/* 封闭性自检：功能宏恒为 0 或 1 */
#if (MEM_FEATURE_DEBUG != 0) && (MEM_FEATURE_DEBUG != 1)
#error "MEM_FEATURE_DEBUG out of range"
#endif

/* ---- 结构常量：不设开关，写死在此处 ---- */
#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4)
#define MEM_ALIGN                       (16U)   /* rv32：psABI 要求 16 B，同时是块粒度 */
#else
#define MEM_ALIGN                       (8U)    /* 宿主 64 位指针：描述符按指针宽度对齐 */
#endif

/* 最小块由 bestfit.h 以 span 口径派生：MEM_MIN_BLOCK = 块头 + 一个对齐粒度。
   不在此处写魔数，避免再出现"0x20 到底是块占位还是数据区"的单位歧义。 */

#endif /* __MEM_FEATURE_H__ */
