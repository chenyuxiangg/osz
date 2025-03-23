#ifndef __QUALIFER_H__
#define __QUALIFER_H__

#define STATIC
#define INLINE
#define SECTION_KERNEL_INIT_DATA        __attribute__((section(".data.kernel.init")))
#define SECTION_KERNEL_INIT_TEXT        __attribute__((section(".text.kernel.init")))
#define ATTR_ALIGN(n)                   __attribute__((aligned((n))))

#define MODULE_INIT_IMPL(symb, level, fn)   VOID *symb __attribute__((section(#level))) = (VOID *)fn;
#define MODULE_INIT(fn, level)          MODULE_INIT_IMPL(init_##fn, .init.init_##level, fn)

#endif