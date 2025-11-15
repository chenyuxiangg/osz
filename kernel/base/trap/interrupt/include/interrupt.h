#ifndef __INTERRUPT_PRI_H__
#define __INTERRUPT_PRI_H__
#include "comm.h"

#define M_SOFTWARE_INTERRUPT            (3)
#define M_TIMER_INTERRUPT               (7)
#define M_EXTERNAL_INTERRUPT            (11)
#define GLOBAL_INTERRUPT_INVALID_NUM    (0x0)

#define HAS_INT_CALLBACK(source_id)       (g_int_infos[(source_id)].func == NULL ? FALSE : TRUE)
#define INT_CALLBACK_ENTITY(source_id)    (g_int_infos[(source_id)].func)
#define INT_IS_TIMER(source_id)           ((source_id) == M_TIMER_INTERRUPT ? TRUE : FALSE)
#define INT_IS_SOFTWARE(source_id)        ((source_id) == M_SOFTWARE_INTERRUPT ? TRUE : FALSE)

typedef void (*interrupt_callback)(void *args);

typedef struct INT_INFO {
    interrupt_callback  func;
    VOID                *args;
} INT_INFO;

UINT32 osz_interrupt_init(VOID);
UINT32 osz_interrupt_regist(UINT32 source_id, interrupt_callback func);
UINT32 osz_interrupt_unregist(UINT32 source_id);
UINT32 osz_interrupt_enable(UINT32 source_id);
UINT32 osz_interrupt_disable(UINT32 source_id);
UINT32 osz_interrupt_set_pri(UINT32 source_id, UINT32 pri);
UINT32 osz_interrupt_set_pri_th(UINT32 pri_th);

#endif