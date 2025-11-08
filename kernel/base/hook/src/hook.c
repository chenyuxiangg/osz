#include "hook.h"

// user can config by hook_table, which will define in target
osz_hook_t g_os_hook;
extern osz_hook_entry_t g_hook_table[HOOK_ID_MAX];

void_t inner_hook_init(void_t)
{
    for (uint32_t index = 0; index < sizeof(g_hook_table)/sizeof(osz_hook_entry_t); ++index) {
        switch (g_hook_table[index].id) {
            case HOOK_ID_PRINT:
                g_os_hook.print = (osz_print_hook_t)g_hook_table[index].callback;
                break;
            default:
                break;
        }
    }
}
MODULE_INIT(inner_hook_init, l1)