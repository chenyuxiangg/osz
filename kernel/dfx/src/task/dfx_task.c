#include "task.h"
#include "hook.h"
#include "shell.h"
#include "dfx_task.h"
#include "printf.h"

// inner API
void_t inner_show_task_info(uint16_t task_id)
{
    uint32_t intsave = 0;
    ARCH_INT_LOCK(intsave);
    if (TASK_STATE(task_id) == TSK_FLAG_FREE) {
        ARCH_INT_UNLOCK(intsave);
        return;
    }
    g_os_hook.print("%#x\t", task_id);
    g_os_hook.print("%s\t", TASK_NAME(task_id));
    g_os_hook.print("%#x\t", TASK_PRIORITY(task_id));
    g_os_hook.print("%#x\t", TASK_STACK_TOP(task_id));
    g_os_hook.print("%#x\t", TASK_STACK_SIZE(task_id));
    g_os_hook.print("%#x\t", TASK_STACK_SP(task_id));
    g_os_hook.print("%#x\n", TASK_STATE(task_id));
    ARCH_INT_UNLOCK(intsave);
}

// extern API
void_t dfx_get_task_info(uint32_t argc, uint8_t **argv[])
{
    if (argc > 1) {
        return;
    }
    uint32_t task_id = strtoul((const char*)argv[0], NULL, 16);
    uint32_t start_task_id = task_id;
    uint32_t end_task_id = start_task_id + 1;
    if ((task_id != DFX_TASK_ALL_TASK) && (task_id >= OSZ_CFG_TASK_LIMIT)) {
        return;
    }
    if (task_id == DFX_TASK_ALL_TASK) {
        start_task_id = 0;
        end_task_id = OSZ_CFG_TASK_LIMIT;
    }
    if (g_os_hook.print == NULL) {
        return;
    }
    g_os_hook.print("\n");
    g_os_hook.print("id\tname\tpri\tstack_top\tstack_size\tsp\tstatue\n");

    for (uint32_t id = start_task_id; id < end_task_id; ++id) {
        inner_show_task_info(id);
    }
}

REGISTER_STATIC_CMD(task, (CMD_CALLBACK_FUNC)dfx_get_task_info, 1)