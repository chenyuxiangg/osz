#ifndef __INNER_SHELL_ERR_H__
#define __INNER_SHELL_ERR_H__

#define SHELL_BASE_ERR                          (0x0)
#define SHELL_BUFFER_FULL_ERR                   (SHELL_BASE_ERR | 0x1)
#define SHELL_CMD_KEY_OVER_LEN_ERR              (SHELL_BASE_ERR | 0x2)
#define SHELL_PARAMS_NAME_INVALID_ERR           (SHELL_BASE_ERR | 0x3)
#define SHELL_PARAMS_CALLBACK_INVALID_ERR       (SHELL_BASE_ERR | 0x4)

#endif