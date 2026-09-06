#ifndef __ASSERT_H__
#define __ASSERT_H__

/*
 * Host-side stub for OSZ's assert.h.
 *
 * The OSZ assert.h defines:
 *     #define ASSERT(x)  ({ if ((x) == NULL) assert(); })
 *     void assert();
 * where `assert()` is implemented in `kernel/comm/src/assert.S` for the
 * bare-metal RISC-V target.
 *
 * When compiling OSZ source files on the host (x86 / Linux) for unit tests,
 * we want ASSERT failures to abort the test process so gtest reports the
 * failure instead of trapping the CPU.
 *
 * This header is placed in `test/include/` and added to the include path
 * BEFORE `kernel/comm/include/` so it shadows the OSZ version.
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * ASSERT(x) — OSZ semantics: pass when x is non-NULL / non-zero.
 * On failure, print a diagnostic and abort.
 */
#define ASSERT(x)                                                          \
    do {                                                                   \
        if (!(x)) {                                                        \
            fprintf(stderr,                                                \
                    "ASSERT failed: %s at %s:%d\n",                         \
                    #x, __FILE__, __LINE__);                                \
            fflush(stderr);                                                \
            abort();                                                       \
        }                                                                  \
    } while (0)

#endif /* __ASSERT_H__ */