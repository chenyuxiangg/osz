#include "comm.h"
#include "exception.h"
#include "csr.h"

UINT32 exception_handler(UINT32 mcause, UINT32 mepc)
{
    printf("mcause: %#x, mepc: %#x\n", mcause, mepc);
    printf("mtval: %#x\n", CSR_READ(mtval));
    return (mepc);
}