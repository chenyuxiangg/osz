#ifndef __FLOAT_H__
#define __FLOAT_H__

/*
 * Host-side stub for OSZ's float.h.
 *
 * OSZ's float.h hardcodes FLT/DOUBLE/LDBL constants and uses the
 * `__FLOAT_H__` guard. To avoid colliding with the system header's
 * declarations, we forward to the system header and rely on the guard
 * to skip OSZ's float.h when comm.h reaches for it later.
 */

#include <float.h>

#endif /* __FLOAT_H__ */