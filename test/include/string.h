#ifndef __STRING_H__
#define __STRING_H__

/*
 * Host-side stub for OSZ's string.h.
 *
 * OSZ's string.h declares memset / memcpy / strlen / strncmp / strncpy
 * using the OSZ type names (UINT32, size_t, etc.). The implementations
 * live elsewhere in the kernel and are not part of the host-test scope.
 *
 * For tests we use the system libc's string.h so the standard
 * functions are available directly. The `__STRING_H__` guard prevents
 * OSZ's string.h from being included later via comm.h.
 */

#include <string.h>

#endif /* __STRING_H__ */