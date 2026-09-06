#ifndef __STDBOOL_H__
#define __STDBOOL_H__

/*
 * Host-side stub for OSZ's stdbool.h.
 *
 * The OSZ header is intentionally minimal (it only defines `bool`,
 * `true`, `false` when compiling C, since OSZ's ztype.h already
 * provides its own BOOL type). On the host we don't need anything
 * additional — C++ has `bool` built in, and the OSZ code we test uses
 * OSZ's BOOL rather than C's `_Bool`.
 *
 * Defining the guard is enough; we deliberately do NOT include any
 * system header to avoid conflicting with C++'s built-in `bool`.
 */

#endif /* __STDBOOL_H__ */