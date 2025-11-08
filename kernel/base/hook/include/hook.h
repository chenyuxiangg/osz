#ifndef __HOOK_H__
#define __HOOK_H__

#include "comm.h"

typedef int32_t (*osz_print_hook_t)(const char *format, ...);

typedef struct {
    osz_print_hook_t print;
} osz_hook_t;

typedef enum {
    HOOK_ID_PRINT = 0,
    HOOK_ID_MAX,
} osz_hook_id_t;

typedef struct {
    osz_hook_id_t id;
    uintptr_t callback;
} osz_hook_entry_t;

extern osz_hook_t g_os_hook;

#endif