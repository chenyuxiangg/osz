#ifndef __ARCH_ADAPTER_H__
#define __ARCH_ADAPTER_H__
#include "csr.h"

#define ARCH_INT_LOCK(intSave) do { \
    intSave = CSR_READ(mstatus); \
    CSR_READ_CLEAR(mstatus, MSTATUS_MIE); \
} while(0)

#define ARCH_INT_UNLOCK(intSave) do { \
    CSR_READ_SET(mstatus, intSave); \
} while(0)

#endif