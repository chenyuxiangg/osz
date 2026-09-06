#ifndef __ZTYPE_H__
#define __ZTYPE_H__

/*
 * Host-side stub for OSZ's ztype.h.
 *
 * OSZ's ztype.h re-defines several types that already exist in glibc's
 * <stdint.h> / <stddef.h> (size_t, ptrdiff_t, int8_t, uintptr_t, ...).
 * The OSZ versions use 32-bit widths (because the kernel targets RV32),
 * which collides with the host libc's 64-bit definitions on x86_64.
 *
 * Strategy: provide OSZ's INT/UINT/CHAR/VOID/BOOL typedefs locally (these
 * don't conflict), and rely on the system stdint.h / stddef.h for the
 * fixed-width and size types. OSZ source code that uses INT8 / UINT32 /
 * CHAR / VOID / BOOL gets our local definitions; code that uses size_t /
 * int8_t / intptr_t / uintptr_t gets the system definitions.
 *
 * This header is forced-included (via osz_host_preinclude.h) BEFORE any
 * OSZ source code is processed, so OSZ's own ztype.h is skipped via the
 * shared `__ZTYPE_H__` guard.
 *
 * Caveat: OSZ source that assumes size_t == 32-bit will misbehave at
 * runtime on a 64-bit host. The current test scope (dlink, sortlink,
 * strtoul) does not rely on size_t width.
 */

#include <stddef.h>
#include <stdint.h>

/* OSZ-specific primitive types */
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef long long           INT64;
typedef signed char         INT8;
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
/* OSZ's convention is UINT32 on RV32. On the host (x86_64) we use the
 * pointer-sized unsigned type so the macros that do
 *     (UINTPTR) &((type *)0)->member
 * keep working without truncation errors from C++ -Wpedantic. */
typedef uintptr_t           UINTPTR;
typedef void                VOID;
typedef unsigned char       BOOL;

/* OSZ aliases — these names are not in the C standard library so they
 * can be defined freely without conflict. */
typedef BOOL                bool_t;
typedef VOID                void_t;
typedef uint64_t            uint64_t;
typedef int64_t             int64_t;
typedef uint32_t            uint32_t;
typedef int32_t             int32_t;
typedef uint16_t            uint16_t;
typedef int16_t             int16_t;
typedef uint8_t             uint8_t;
typedef int8_t              int8_t;
typedef intmax_t            intmax_t;   /* use system's */
typedef uintptr_t           uintptr_t;  /* use system's */

/* size_t and ptrdiff_t come from <stddef.h> above — do NOT redefine. */

/* Status / boolean constants — OSZ's local convention. */
#ifndef NULL
#define NULL        (0)
#endif
#ifndef TRUE
#define TRUE        (1)
#endif
#ifndef FALSE
#define FALSE       (0)
#endif
#define SUCCESS     (0)
#define FAILUER     (-1)
#define OS_OK       (0)
#define OS_NOK      (-1)

#define READ_BYTE(addr)             \
    ({ unsigned char _v = *(volatile unsigned char *)(addr); _v; })
#define WRITE_BYTE(addr, val)       (*(volatile unsigned char *)(addr) = (val))

#endif /* __ZTYPE_H__ */