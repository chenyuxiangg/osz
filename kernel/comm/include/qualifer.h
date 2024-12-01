#ifndef __QUALIFER_H__
#define __QUALIFER_H__

#define STATIC
#define SECTION_KERNEL_INIT_DATA        __attribute__((section(".data.kernel.init")))
#define SECTION_KERNEL_INIT_TEXT        __attribute__((section(".text.kernel.init")))
#define ATTR_ALIGN(n)                   __attribute__((aligned((n))))

#endif