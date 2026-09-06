#ifndef __MENUCONFIG_H__
#define __MENUCONFIG_H__

/*
 * Host-side stub for the auto-generated `menuconfig.h`.
 *
 * The OSZ build system generates `targets/platform/include/menuconfig.h`
 * from Kconfig to expose OSZ_CFG_* feature macros to the kernel.
 *
 * When compiling the small subset of OSZ source files under test on the
 * host, none of those OSZ_CFG_* macros are referenced, so this header
 * intentionally defines nothing.
 */

#endif /* __MENUCONFIG_H__ */