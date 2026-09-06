/*
 * Host stub for the bare-metal `assert()` function.
 *
 * `kernel/comm/src/assert.S` implements `void assert()` for the RISC-V
 * target (it traps the CPU on failure). On the host, when compiling OSZ
 * sources that declare `void assert();` in their includes, we provide a
 * linkable definition so binaries can be produced.
 *
 * In the OSZ test build the `ASSERT(x)` macro has been redefined (see
 * `test/include/assert.h`) so production code never calls this bare
 * `assert()` function in practice. The symbol exists only to satisfy
 * any header that declares it. If it is ever called at runtime it simply
 * aborts the process, mirroring the original intent.
 */

#include <stdio.h>
#include <stdlib.h>

void assert(void)
{
    fprintf(stderr, "OSZ assert() invoked on host — aborting.\n");
    fflush(stderr);
    abort();
}