#ifndef __OFFSET_H__
#define __OFFSET_H__
#include "ztype.h"

#define STRUCT_ENTRY(type, member, item)     ((type *)(VOID *)((char *)(item) - OFFSET(type, member)))
#define OFFSET(type, member)  ((UINTPTR) &((type *)0)->member)

#endif