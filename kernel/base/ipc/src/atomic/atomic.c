#include "atomic.h"
#include "arch_adapter.h"

UINT32 atomic_read(ATOMIC *data)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    UINT32 val = (UINT32)*data;
    ARCH_INT_UNLOCK(intSave);
    return val;
}

VOID atomic_write(ATOMIC *data, ATOMIC value)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    *data = value;
    ARCH_INT_UNLOCK(intSave);
}

VOID atomic_inc(ATOMIC *data)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    *data += 1;
    ARCH_INT_UNLOCK(intSave);
}

VOID atomic_dec(ATOMIC *data)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    *data -= 1;
    ARCH_INT_UNLOCK(intSave);
}

VOID atomic_sub(ATOMIC *data, ATOMIC val)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    *data -= val;
    ARCH_INT_UNLOCK(intSave);
}

VOID atomic_add(ATOMIC *data, ATOMIC val)
{
    UINT32 intSave = 0;
    ARCH_INT_LOCK(intSave);
    *data += val;
    ARCH_INT_UNLOCK(intSave);
}