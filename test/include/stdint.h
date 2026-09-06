#ifndef __STDINT_H__
#define __STDINT_H__

/*
 * Host-side stub for OSZ's stdint.h.
 *
 * OSZ's stdint.h uses the uClibc-style `__NEED_*` macros and does not
 * actually expose the intN_t / uintN_t typedefs expected by C99 — it
 * is designed to be combined with a freestanding libc that fills the
 * gaps. On glibc this header causes conflicting-type errors when the
 * compiler pulls in <stdint.h> indirectly via <stdio.h>, etc.
 *
 * Forwarding to the system header resolves all conflicts.
 */

#include <stdint.h>

#endif /* __STDINT_H__ */