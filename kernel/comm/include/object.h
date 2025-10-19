#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "ztype.h"
#include "dlink.h"
#include "qualifer.h"
#include "menuconfig.h"

typedef struct __osz_rt_context_t {
    uint32_t is_on_int: 1;
    uint32_t tid: 8;
    uint32_t hwi: 8;
    uint32_t reserv: 15;
} osz_rt_context_t;

typedef enum __osz_module_enum {
#ifdef OSZ_CFG_INTERRUPT
    OSZ_MOD_INTERRUPT,
#endif
#ifdef OSZ_CFG_MEM
    OSZ_MOD_MEMORY,
#endif
#ifdef OSZ_CFG_TASK
    OSZ_MOD_TASK,
#endif
#ifdef OSZ_CFG_EVENT
    OSZ_MOD_EVENTS,
#endif
#ifdef OSZ_CFG_SEM
    OSZ_MOD_SEM,
#endif
#ifdef OSZ_CFG_MUTEX
    OSZ_MOD_MUTEX,
#endif
#ifdef OSZ_CFG_SOFTTIMER
    OSZ_MOD_SWT,
#endif
    OSZ_MOD_MAX
} osz_module_enum_t;

typedef struct __osz_module_t {
    uint32_t id: 8;
    uint32_t reserv: 24;
    osz_list_t used_obj_list;
    osz_list_t free_obj_list;
} osz_module_t;

typedef struct __osz_obj_t {
    osz_list_t obj_list;              // 挂载到used_obj_list或者free_static_obj_list_head
    uint8_t owner;                    // 中断上下文，记录中断号；任务上下文，记录任务ID；启动阶段，记录MAX_TASK_NUM + 1
    osz_module_t *module;
    uint8_t name[OSZ_CFG_OBJ_NAME_MAX_LENS];
} osz_obj_t;

#define OBJECT_BASE_ERR                             (0x00010000)
#define OBJECT_MOD_EMPTY_ERR                        (OBJECT_BASE_ERR | 0x1)
#define OBJECT_MOD_ID_OVER_ERR                      (OBJECT_BASE_ERR | 0x2)
#define OBJECT_IS_NULL_ERR                          (OBJECT_BASE_ERR | 0x3)
#define OBJECT_FREE_OBJ_OUTTER_NOT_NULL_ERR         (OBJECT_BASE_ERR | 0x4)
#define OBJECT_FREE_OBJ_ALL_USED_ERR                (OBJECT_BASE_ERR | 0x5)

#define FREE_OBJ_NODE(mod_id)                       (g_modules[(mod_id)].free_obj_list.next)
#define MODULE_FREE_LIST_HEAD(mod_id)               (&(g_modules[(mod_id)].free_obj_list))
#define CURRENT_RT_CONTEXT                          (g_rt_context.is_on_int == 0 ? g_rt_context.tid : g_rt_context.hwi)

uint32_t object_init(osz_obj_t *obj, osz_module_enum_t module_id, uint8_t *name, uint8_t name_size);
uint32_t get_free_obj(osz_module_enum_t module_id, osz_obj_t **outter_obj);

extern osz_module_t g_modules[OSZ_MOD_MAX];

#endif