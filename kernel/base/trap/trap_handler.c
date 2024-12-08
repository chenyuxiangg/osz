#include "printf.h"
#include "comm.h"
#include "exception.h"

#define INTERRUPT_MASK          (0x80000000)
#define IS_INTERRUPT(mcause)    (((mcause) & INTERRUPT_MASK) == INTERRUPT_MASK ? TRUE : FALSE)

UINT32 trap_handler(UINT32 mcause, UINT32 mepc)
{
    if (!IS_INTERRUPT(mcause)) {
        mepc = exception_handler(mcause, mepc);
    }
    return mepc;
}