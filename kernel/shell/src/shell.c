#include "task.h"
#include "mem.h"
#include "string.h"
#include "shell.h"
#include "inner_shell_err.h"

#define SHELL_NAME                  "OSZ"
#define SHELL_PRINT                 printf
#define SHELL_CMD_KEY_LEN_MAX       (0x16)

#define SHELL_SPECIAL_CHAR_DEL      (0x7F)          // Delete
#define SHELL_SPECIAL_CHAR_HT       (0x9)           // Horizental Tab
#define SHELL_SPECIAL_CHAR_CR       (0xD)           // Carriage Return
#define SHELL_SPECIAL_CHAR_ESC      (0x1B)          // ESC
#define SHELL_SPECIAL_CHAR_LSB      (0x5B)          // Left Square Bracket
#define SHELL_SPECIAL_CHAR_A        (0x41)          // A, ESC + LSB + A is up arrow
#define SHELL_SPECIAL_CHAR_B        (0x42)          // B, ESC + LSB + B is down arrow
#define SHELL_SPECIAL_CHAR_C        (0x43)          // C, ESC + LSB + C is left arrow
#define SHELL_SPECIAL_CHAR_D        (0x44)          // D, ESC + LSB + D is right arrow
#define SHELL_SPECIAL_CHAR_SPACE    (0x20)          // Space

STATIC CMD_NODE g_cmd_head;
STATIC SHELL_CB g_shell_cb;

STATIC VOID inner_shell_init(VOID)
{
    dlink_init(&(g_cmd_head.list));
    g_shell_cb.shell_capcity = SHELL_BUFFER_MAX_NUM - 1;    // SHELL_BUFFER_MAX_NUM reserve for '\0'
    g_shell_cb.shell_state = SHELL_STATE_NONE;
}
MODULE_INIT(inner_shell_init, l4)

STATIC BOOL inner_shell_buf_is_full()
{
    return (g_shell_cb.buf_cur_size >= g_shell_cb.shell_capcity);
}

STATIC BOOL inner_shell_char_is_special(CHAR c)
{
    return ((c == SHELL_SPECIAL_CHAR_DEL) ||
            (c == SHELL_SPECIAL_CHAR_HT) ||
            (c == SHELL_SPECIAL_CHAR_CR) ||
            (c == SHELL_SPECIAL_CHAR_ESC) ||
            (c == SHELL_SPECIAL_CHAR_LSB));
}

STATIC UINT32 inner_shell_get_cmd_key_count_by_tab()
{
    return 0;
}

STATIC VOID inner_shell_get_cmd_key_by_tab(UCHAR *keys[])
{

}

STATIC VOID inner_shell_uarrow_key_do()
{

}

STATIC VOID inner_shell_darrow_key_do()
{

}

STATIC VOID inner_shell_larrow_key_do()
{

}

STATIC VOID inner_shell_rarrow_key_do()
{

}

STATIC VOID inner_shell_tab_key_do(VOID)
{
    UINT32 count = inner_shell_get_cmd_key_count_by_tab();
    if (count == 0) {
        return;
    }
    UCHAR **keys = (UCHAR **)osz_zalloc(sizeof(UCHAR *) * count);
    inner_shell_get_cmd_key_by_tab(keys);
    if (count == 1) {
        g_shell_cb.buf_cur_size = strlen(keys);
        memcpy((VOID *)g_shell_cb.buf, (VOID *)keys, (size_t)g_shell_cb.buf_cur_size);
        for (UINT32 i = g_shell_cb.buf_cur_size; i < g_shell_cb.buf_cur_size; ++i) {
            SHELL_PRINT("%c", g_shell_cb.buf[i]);
        }
        return osz_free((VOID *)keys);
    }
    SHELL_PRINT("\n");
    for (UINT32 i = 0; i < SHELL_CMD_KEY_LEN_MAX * count; i += SHELL_CMD_KEY_LEN_MAX) {
        SHELL_PRINT("%s ", keys + i);
    }
    SHELL_PRINT("\n");
    SHELL_PRINT("%s$ %s", SHELL_NAME, g_shell_cb.buf);
    return osz_free((VOID *)keys);
}

STATIC VOID inner_shell_deal_get_phase()
{
    if (inner_shell_buf_is_full()) {
        g_shell_cb.shell_state = SHELL_STATE_ERR;
        g_shell_cb.shell_err_code = SHELL_BUFFER_FULL_ERR;
        return;
    }
    if (inner_shell_char_is_special(g_shell_cb.shell_cur_char)) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        return;
    }
    
    g_shell_cb.buf[g_shell_cb.buf_cur_size++] = (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_SPACE) ? '0' : g_shell_cb.shell_cur_char;
    g_shell_cb.shell_state = SHELL_STATE_NONE;
}

STATIC VOID inner_shell_deal_switch_phase()
{
    if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_DEL) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        g_shell_cb.buf_cur_size--;
        SHELL_PRINT("\b ");
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_HT) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_tab_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_CR) {
        g_shell_cb.shell_state = SHELL_STATE_PARSE;
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_ESC) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_LSB) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_A) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_uarrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_B) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_darrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_C) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_larrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_D) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_rarrow_key_do();
    }
    return;
}

STATIC VOID inner_shell_deal_parse_phase()
{
    CHAR *name[SHELL_CMD_KEY_LEN_MAX] = { 0 };
    CHAR **args = NULL;
    CHAR *buf_ptr = g_shell_cb.buf;
    CHAR *buf_end = g_shell_cb.buf + g_shell_cb.buf_cur_size;
    if (strlen(g_shell_cb.buf) >= SHELL_CMD_KEY_LEN_MAX) {
        g_shell_cb.shell_state = SHELL_STATE_ERR;
        g_shell_cb.shell_err_code = SHELL_CMD_KEY_OVER_LEN_ERR;
        memset(g_shell_cb.buf, 0, g_shell_cb.shell_capcity);
        return;
    }
    memcpy((VOID *)name, (VOID *)buf_ptr, (size_t)strlen(g_shell_cb.buf));
    buf_ptr += strlen(g_shell_cb.buf) + 1;

    DLINK_NODE *iter = NULL;
    CMD_NODE *cmd = NULL;
    DLINK_FOREACH(iter, &(g_cmd_head.list)) {
        cmd = STRUCT_ENTRY(CMD_NODE, list, iter);
        if (strncmp(cmd->cmd_name, name, strlen(name)) == 0) {
            break;
        }
    }
    if (iter != &(g_cmd_head.list)) {
        if (cmd->argc != 0) {
            args = osz_malloc(sizeof(UCHAR *) * cmd->argc);
            UINT32 index = 0;
            for (; buf_ptr < buf_end; buf_ptr += strlen(buf_ptr)) {
                args[index++] = buf_ptr;
            }
        }
        cmd->args = args;
    } else {
        cmd = NULL;
    }
    g_shell_cb.cur_cmd = cmd;
    memset(g_shell_cb.buf, 0, g_shell_cb.shell_capcity);
    g_shell_cb.shell_state = SHELL_STATE_EXC;
}

STATIC VOID inner_shell_deal_exc_phase()
{
    g_shell_cb.shell_state = SHELL_STATE_NONE;
    CMD_NODE *cmd = g_shell_cb.cur_cmd;
    if (cmd != NULL) {
        cmd->cmd_func(cmd->argc, cmd->args);
    } else {
        SHELL_PRINT("No such command.\n");
    }
    
    if (cmd->args != NULL) {
        osz_free(cmd->args);
    }
    cmd->args = NULL;
}

STATIC VOID inner_shell_deal_err_phase()
{
    memset(g_shell_cb.buf, 0, g_shell_cb.shell_capcity);
    g_shell_cb.buf_cur_size = 0;
    // g_shell_cb.shell_err_code = '\0';
    g_shell_cb.shell_state = SHELL_STATE_NONE;
}

STATIC UINT32 inner_shell_check_params(CMD_PARAMS *params)
{
    if (strlen(params->cmd_name) == 0) {
        return SHELL_PARAMS_NAME_INVALID_ERR;
    }
    if (params->cmd_func == NULL) {
        return SHELL_PARAMS_CALLBACK_INVALID_ERR;
    }
    return OS_OK;
}

VOID shell_register_cmd(CMD_PARAMS *params)
{
    ASSERT(params);
    if (inner_shell_check_params(params) != OS_OK) {
        return;
    }
    CMD_NODE *cmd = (CMD_NODE *)osz_zalloc(sizeof(CMD_NODE));
    cmd->argc = params->argc;
    cmd->cmd_func = params->cmd_func;
    cmd->cmd_name = params->cmd_name;
    dlink_insert_tail(&(g_cmd_head.list), &(cmd->list));
}

VOID shell_loop()
{
    SHELL_PRINT("%s$ ", SHELL_NAME);
    while (TRUE) {
        switch (g_shell_cb.shell_state) {
            case SHELL_STATE_NONE:
                break;
            case SHELL_STATE_GET:
                inner_shell_deal_get_phase();
                break;
            case SHELL_STATE_SWITCH:
                inner_shell_deal_switch_phase();
                break;
            case SHELL_STATE_PARSE:
                inner_shell_deal_parse_phase();
                break;
            case SHELL_STATE_EXC:
                inner_shell_deal_exc_phase();
                SHELL_PRINT("%s$ ", SHELL_NAME);
                break;
            case SHELL_STATE_ERR:
                inner_shell_deal_err_phase();
                SHELL_PRINT("err-code: %#x\n", g_shell_cb.shell_err_code);
                SHELL_PRINT("%s$ ", SHELL_NAME);
                break;
            default:
                break;
        }
        os_udelay(5);
    }
}

SHELL_STATE shell_get_state(VOID)
{
    return g_shell_cb.shell_state;
}

VOID shell_set_state(SHELL_STATE state)
{
    g_shell_cb.shell_state = state;
}

VOID shell_set_current_char(CHAR c)
{
    g_shell_cb.shell_cur_char = c;
}