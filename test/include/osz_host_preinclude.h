#ifndef __OSZ_HOST_PREINCLUDE_H__
#define __OSZ_HOST_PREINCLUDE_H__

/*
 * osz_host_preinclude.h — host-side test build pre-include header.
 *
 * Several OSZ headers (`assert.h`, `stdarg.h`, `stdint.h`, `stdbool.h`,
 * `float.h`, `string.h`, `printf.h`, `menuconfig.h`) are written for a
 * bare-metal / newlib target and either redefine host libc symbols or
 * conflict with glibc. Rather than rewriting the OSZ source tree, we
 * force-include this file at the top of every translated unit so the
 * host-side stubs in `test/include/` set the matching include guards
 * *before* `kernel/comm/include/comm.h` pulls them in via `#include
 * "..."`. The guards cause the OSZ copies to be skipped wholesale.
 *
 * Each header below lives in `test/include/` and (where appropriate)
 * forwards to the system header so callers get the real definitions.
 *
 * Order matters: the system stdarg.h / stdint.h are pulled in FIRST,
 * because the host-side stubs further down include <stdio.h> which
 * depends on `__gnuc_va_list` being defined.
 */

#include <stdarg.h>
#include <stdint.h>

#include "assert.h"
#include "stdarg.h"
#include "stdint.h"
#include "stdbool.h"
#include "float.h"
#include "string.h"
#include "printf.h"
#include "menuconfig.h"
#include "ztype.h"

#endif /* __OSZ_HOST_PREINCLUDE_H__ */