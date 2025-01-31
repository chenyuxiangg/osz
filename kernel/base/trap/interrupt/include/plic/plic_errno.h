#ifndef __PLIC_ERRNO_H__
#define __PLIC_ERRNO_H__

#define PLIC_MODULE_ID                  (0x00000000)
#define PLIC_ERRNO_INVALID_SOURCE_ID    (PLIC_MODULE_ID | 0x1)
#define PLIC_ERRNO_SOURCE_ID_IS_ZERO    (PLIC_MODULE_ID | 0x2)

#endif