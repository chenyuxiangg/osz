#ifndef __STDARG_H__
#define __STDARG_H__

/*
 * Host-side stub for OSZ's stdarg.h.
 *
 * The OSZ stdarg.h redefines va_start/va_end/va_arg/va_copy with
 * __builtin_va_*. That is fine on bare-metal, but on glibc the file
 * collides with the system's <stdarg.h> (__gnuc_va_list, etc.). We
 * forward to the system header so that any varargs code compiled in
 * tests just works.
 */

#include <stdarg.h>

#endif /* __STDARG_H__ */