#ifndef __SHELL_H__
#define __SHELL_H__
#include "comm.h"
#include "dlink.h"

#define SHELL_BUFFER_MAX_NUM        (0x400)

typedef void (*CMD_CALLBACK_FUNC)(UINT32 argc, CHAR *argv[]);

typedef enum {
    SHELL_STATE_NONE = 0,
    SHELL_STATE_GET,
    SHELL_STATE_SWITCH,
    SHELL_STATE_PARSE,
    SHELL_STATE_EXC,
    SHELL_STATE_ERR,
} SHELL_STATE;

typedef struct {
    DLINK_NODE          list;
    CHAR               *cmd_name;
    CMD_CALLBACK_FUNC   cmd_func;
    CHAR               **args;
    UINT16              max_argc;
    UINT16              cur_argc;
} CMD_NODE;

typedef struct {
    CHAR               *cmd_name;
    CMD_CALLBACK_FUNC   cmd_func;
    UINT32              argc;
} CMD_PARAMS;

typedef struct {
    CHAR buf[SHELL_BUFFER_MAX_NUM];
    UINT32 buf_cur_size : 10;
    UINT32 shell_capcity : 11;
    union {
        UINT32 shell_err_code : 8;
        UINT32 shell_cur_char : 8;
    };
    UINT32 shell_state : 3;
    CMD_NODE *cur_cmd;
} SHELL_CB;

VOID shell_register_cmd(CMD_PARAMS *params);
VOID shell_loop();
SHELL_STATE shell_get_state(VOID);
VOID shell_set_state(SHELL_STATE state);
VOID shell_set_current_char(CHAR c);

#endif