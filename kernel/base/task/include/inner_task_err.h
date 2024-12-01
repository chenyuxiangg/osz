#ifndef __INNER_TASK_ERR_H__
#define __INNER_TASK_ERR_H__

#define TASK_BASE_ERR                       (0x10)
#define TASK_CREATE_CHECK_NAME_ERR          (TASK_BASE_ERR | 0x1)
#define TASK_CREATE_CHECK_STACK_AGLIGN_ERR  (TASK_BASE_ERR | 0x2)
#define TASK_CREATE_CHECK_STACK_SIZE_ERR    (TASK_BASE_ERR | 0x3)

#endif
