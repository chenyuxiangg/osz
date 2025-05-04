#ifndef __SHELL_H__
#define __SHELL_H__
#include "menuconfig.h"
#include "comm.h"
#include "dlink.h"
#include "fifo.h"

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
    CHAR *history_cmds[OSZ_CFG_SHELL_HISTORY_CMD_NUM];
    UINT32 history_cmd_num : 8;
    UINT32 history_cursor : 8;
    UINT32 history_has_cmd : 1;
    UINT32 history_max_cmd_len : 7;
    UINT32 reserv : 8;
} CMD_HISTORY;

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
    FIFO *fifo;
    UINT32 buf_cur_size : 10;
    UINT32 shell_capcity : 11;
    union {
        UINT32 shell_err_code : 8;
        UINT32 shell_cur_char : 8;
    };
    UINT32 shell_state : 3;
    CHAR *shell_buf_cursor;
    CMD_NODE *cur_cmd;
} SHELL_CB;

VOID shell_register_cmd(CMD_PARAMS *params);
VOID shell_loop();
SHELL_STATE shell_get_state(VOID);
VOID shell_set_state(SHELL_STATE state);
VOID shell_write_fifo(CHAR c);
UINT32 shell_read_fifo(VOID);

#endif