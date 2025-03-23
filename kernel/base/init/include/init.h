#ifndef __INIT_H__
#define __INIT_H__
#include "comm.h"
typedef VOID (*init_func_type)(VOID);

VOID os_init(VOID);

#endif