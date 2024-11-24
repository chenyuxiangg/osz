#ifndef __OFFSET_H__
#define __OFFSET_H__
#include "ztype.h"

#define OFFSET(TYPE, MEMBER)  ((UINTPTR) &((TYPE *)0)->MEMBER)

#endif