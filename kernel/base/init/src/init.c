#include "comm.h"
#include "init.h"

extern UINT32 __init_start;
extern UINT32 __init_end;
extern VOID main(INT32 argc, CHAR *argv[]);

#define init_start      (&__init_start)
#define init_end        (&__init_end)

VOID os_init(VOID)
{
    for(UINT32 *func_addr = init_start; func_addr < init_end; ++func_addr) {
        init_func_type func = (init_func_type)(*func_addr);
        if (func != NULL) {
            func();
        }
    }
    main(0, NULL);
}