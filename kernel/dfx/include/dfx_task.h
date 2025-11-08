#ifndef __DFX_TASK_H__
#define __DFX_TASK_H__

#include "comm.h"

#define DFX_TASK_ALL_TASK       (0xffffffff)

void_t dfx_get_task_info(uint32_t argc, uint8_t **argv[]);

#endif