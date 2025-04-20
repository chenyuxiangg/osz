#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include "comm.h"

typedef UINT32 ATOMIC;

UINT32  atomic_read(ATOMIC *data);
VOID    atomic_write(ATOMIC *data, ATOMIC value);
VOID    atomic_inc(ATOMIC *data);
VOID    atomic_dec(ATOMIC *data);
VOID    atomic_sub(ATOMIC *data, ATOMIC val);
VOID    atomic_add(ATOMIC *data, ATOMIC val);

#endif