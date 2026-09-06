#ifndef _PRINTF_H_
#define _PRINTF_H_

/*
 * Host-side stub for the OSZ `printf.h` (Marco Paland's tiny printf).
 *
 * The OSZ version declares its own `printf_()`, `sprintf_()`, etc. via
 * `#define printf printf_` so it can coexist with newlib. To compile OSZ
 * sources that call `printf()` on the host we instead fall back to the
 * standard library.
 */

#include <stdio.h>

/*
 * No macro renaming: keep `printf`, `sprintf`, `snprintf`, `vsnprintf`,
 * `vprintf` as the standard library names. OSZ source code that calls
 * `printf(...)` therefore resolves to stdio's `printf`.
 */

#endif /* _PRINTF_H_ */