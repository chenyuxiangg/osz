#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include "comm.h"
#include "arch_adapter.h"

#define atomic_read(type, pdata) ({    \
    UINT32 intSave = 0;                 \
    ARCH_INT_LOCK(intSave);             \
    type val = *(type *)(pdata);          \
    ARCH_INT_UNLOCK(intSave);           \
    (val);                         \
})

#define atomic_write(type, pdata, value) do { \
    UINT32 intSave = 0;                 \
    ARCH_INT_LOCK(intSave);             \
    *(type *)(pdata) = (value);             \
    ARCH_INT_UNLOCK(intSave);           \
} while(0)

#define atomic_inc(type, pdata) do {    \
    UINT32 intSave = 0;                 \
    ARCH_INT_LOCK(intSave);             \
    *(type *)(pdata) += 1;                \
    ARCH_INT_UNLOCK(intSave);           \
} while(0)

#define atomic_dec(type, pdata) do {    \
    UINT32 intSave = 0;                 \
    ARCH_INT_LOCK(intSave);             \
    *(type *)(pdata) -= 1;                \
    ARCH_INT_UNLOCK(intSave);           \
} while(0)

#define atomic_sub(type, pdata, val) do {   \
    UINT32 intSave = 0;                     \
    ARCH_INT_LOCK(intSave);                 \
    *(type *)(pdata) -= (val);                  \
    ARCH_INT_UNLOCK(intSave);               \
} while(0)

#define atomic_add(type, pdata, val) do {   \
    UINT32 intSave = 0;                     \
    ARCH_INT_LOCK(intSave);                 \
    *(type *)(pdata) += (val);                  \
    ARCH_INT_UNLOCK(intSave);               \
} while(0)

#endif