#ifndef __INTERRUPT_PRI_H__
#define __INTERRUPT_PRI_H__
#include "comm.h"

#define M_SOFTWARE_INTERRUPT            (1 << 3)
#define M_TIMER_INTERRUPT               (1 << 7)
#define M_EXTERNAL_INTERRUPT            (1 << 11)
#define GLOBAL_INTERRUPT_INVALID_NUM    (0x0)
#define LOCAL_INTERRUPT_MAX_NUM         (16)

#define HAS_INT_CALLBACK(source_id)       (g_int_infos[(source_id)].func == NULL ? FALSE : TRUE)
#define INT_CALLBACK_ENTITY(source_id)    (g_int_infos[(source_id)].func)
#define INT_IS_LOCAL(source_id)           ((source_id) >= LOCAL_INTERRUPT_MAX_NUM ? FALSE : TRUE)

typedef void (*interrupt_callback)(void *args);

typedef struct INT_INFO {
    interrupt_callback  func;
    VOID                *args;
} INT_INFO;

UINT32 osz_interrupt_init(VOID);
UINT32 osz_interrupt_regist(UINT32 source_id, interrupt_callback func);
UINT32 osz_interrupt_unregist(UINT32 source_id);
UINT32 osz_interrupt_enable(UINT32 source_id);
UINT32 osz_interrupt_set_pri(UINT32 source_id, UINT32 pri);
UINT32 osz_interrupt_set_pri_th(UINT32 pri_th);

#endif