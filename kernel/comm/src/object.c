#include "object.h"
#include "string.h"

osz_module_t g_modules[OSZ_MOD_MAX];
STATIC osz_rt_context_t g_rt_context = { 0 };

STATIC uint32_t module_init(void_t)
{
    uint32_t mod = 0;
    for (; mod < OSZ_MOD_MAX; ++mod) {
        g_modules[mod].id = mod;
        dlink_init(&(g_modules[mod].used_obj_list));
        dlink_init(&(g_modules[mod].free_obj_list));
    }
    if (mod == 0) {
        return OBJECT_MOD_EMPTY_ERR;
    }
    return OS_OK;
}
MODULE_INIT(module_init, l0)

uint32_t object_init(osz_obj_t *obj, osz_module_enum_t module_id, uint8_t *name, uint8_t name_size)
{
    if (obj == NULL) {
        return OBJECT_IS_NULL_ERR;
    }
    if (module_id >= OSZ_MOD_MAX) {
        return OBJECT_MOD_ID_OVER_ERR;
    }
    if (name != NULL && name_size != 0) {
        memcpy(obj->name, name, name_size);
    }
    dlink_init(&(obj->obj_list));
    obj->owner = CURRENT_RT_CONTEXT;
    obj->module = &(g_modules[module_id]);
    return OS_OK;
}

uint32_t get_free_obj(osz_module_enum_t module_id, osz_obj_t **outter_obj)
{
    if (*outter_obj != NULL) {
        return OBJECT_FREE_OBJ_OUTTER_NOT_NULL_ERR;
    }
    if (module_id >= OSZ_MOD_MAX) {
        return OBJECT_MOD_ID_OVER_ERR;
    }
    osz_list_t *node = FREE_OBJ_NODE(module_id);
    if (node == MODULE_FREE_LIST_HEAD(module_id)) {
        *outter_obj = NULL;
        return OBJECT_FREE_OBJ_ALL_USED_ERR;
    }
    *outter_obj = (osz_obj_t *)node;
    dlink_del_node(&((*outter_obj)->obj_list));
    return OS_OK;
}