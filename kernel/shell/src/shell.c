#include "task.h"
#include "mem.h"
#include "string.h"
#include "shell.h"
#include "inner_shell_err.h"

extern UINT32 __cmd_start;
extern UINT32 __cmd_end;
UINT32 g_cmd_start = (UINT32)&__cmd_start;
UINT32 g_cmd_end = (UINT32)&__cmd_end;

#define SHELL_NAME                  "OSZ"
#define SHELL_NAME_SIZE             (sizeof(SHELL_NAME)+2)
#define SHELL_CMD_KEY_LEN_MAX       (0x16)
#define SHELL_FIFO_MAX_SIZE         (0x40)
#define SHELL_FIFO_READ_VALID       (0x1)
#define SHELL_CMD_CURSOR_INVALID    (255)

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

#define SHELL_PRINT                 printf

STATIC CMD_NODE g_cmd_head;
STATIC SHELL_CB g_shell_cb;
STATIC CMD_HISTORY g_shell_history;

STATIC VOID inner_shell_task_create(VOID)
{
    UCHAR *task_stack = osz_malloc(0x800);
    osz_task_params_t params = {
        .name = "shell_task",
        .stack_attr = STACK_MEM_DYNAMIC,
        .stack_base = (UINTPTR)task_stack,
        .stack_size = 0x800,
        .priority = 0x9,
        .thread = (task_callback_t)shell_loop,
        .data = NULL
    };
    UINT16 task_id;
    UINT32 ret = osz_create_task(&task_id, &params);
    if (ret != OS_OK) {
        printf("ret: %#x\n", ret);
        return;
    }
    osz_task_resume(task_id);
}

STATIC VOID inner_shell_register_system_cmd(VOID)
{
    for (UINT32 addr = g_cmd_start; addr < g_cmd_end; addr += sizeof(CMD_PARAMS)) {
        CMD_PARAMS *cmd = (CMD_PARAMS *)addr;
        shell_register_cmd(cmd);
    }
}

STATIC VOID inner_shell_init(VOID)
{
    dlink_init(&(g_cmd_head.list));
    memset((VOID *)(g_shell_history.history_cmds), 0, sizeof(g_shell_history.history_cmds));
    memset((VOID *)(g_shell_cb.buf), 0, sizeof(g_shell_cb.buf));
    g_shell_cb.fifo = fifo_create(SHELL_FIFO_MAX_SIZE);
    g_shell_cb.shell_capcity = SHELL_BUFFER_MAX_NUM - 1;    // SHELL_BUFFER_MAX_NUM reserve for '\0'
    g_shell_cb.shell_state = SHELL_STATE_NONE;
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf;
    g_shell_history.history_cmd_num = 0;
    g_shell_history.history_cursor = SHELL_CMD_CURSOR_INVALID;
    g_shell_history.history_has_cmd = 0;
    g_shell_history.history_max_cmd_len = 0;
    inner_shell_task_create();
    inner_shell_register_system_cmd();
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

STATIC BOOL inner_shell_buf_cursor_has_shifted(VOID)
{
    return (g_shell_cb.shell_buf_cursor != (g_shell_cb.buf + g_shell_cb.buf_cur_size));
}

STATIC VOID inner_shell_reprint_with_cursor_shift(UINT32 gap_size)
{
    SHELL_PRINT("%s", g_shell_cb.shell_buf_cursor);
    for (UINT32 i = 0; i < gap_size; ++i) {
        SHELL_PRINT("\b");
    }
}

STATIC VOID inner_shell_change_space_to_zero(VOID)
{
    CHAR *ptr = g_shell_cb.buf;
    while (*ptr != '\0') {
        if (*ptr == SHELL_SPECIAL_CHAR_SPACE) {
            *ptr = '\0';
        }
        ptr++;
    }
}

STATIC VOID inner_shell_record_history_cmd(VOID)
{
    if (strlen(g_shell_cb.buf) == 0) {
        return;
    }
    if (g_shell_history.history_has_cmd != 0) {
        UINT32 pre_history_num = g_shell_history.history_cmd_num == 0 ? OSZ_CFG_SHELL_HISTORY_CMD_NUM : g_shell_history.history_cmd_num - 1;
        CHAR *history_cmd = g_shell_history.history_cmds[pre_history_num];
        if ((strlen(g_shell_cb.buf) == strlen(history_cmd)) && 
            (strncmp((VOID *)g_shell_cb.buf, (VOID *)history_cmd, strlen(history_cmd)) == 0)) {
            return;
        }
    }

    if (g_shell_history.history_cmd_num >= OSZ_CFG_SHELL_HISTORY_CMD_NUM) {
        g_shell_history.history_cmd_num %= OSZ_CFG_SHELL_HISTORY_CMD_NUM;
        osz_free(g_shell_history.history_cmds[g_shell_history.history_cmd_num]);
        g_shell_history.history_cmds[g_shell_history.history_cmd_num] = NULL;
    }
    UINT32 cur_cmd_len = strlen(g_shell_cb.buf);
    g_shell_history.history_max_cmd_len = (g_shell_history.history_max_cmd_len < cur_cmd_len) ? cur_cmd_len : g_shell_history.history_max_cmd_len;
    CHAR *history = (CHAR *)osz_zalloc(cur_cmd_len + 1);
    memcpy((VOID *)history, (VOID *)g_shell_cb.buf, cur_cmd_len);
    g_shell_history.history_cmds[g_shell_history.history_cmd_num++] = history;
    g_shell_history.history_has_cmd = 1;
}

STATIC VOID inner_shell_clean_line(VOID) {
    for (UINT32 i = 0; i < g_shell_history.history_max_cmd_len + SHELL_NAME_SIZE; ++i) {
        SHELL_PRINT(" ");
    }
}

STATIC UINT32 inner_shell_get_cmd_key_count_by_tab()
{
    UINT32 cmd_cnt = 0;
    DLINK_NODE *iter = NULL;
    DLINK_FOREACH(iter, &(g_cmd_head.list)) {
        cmd_cnt++;
    }
    return cmd_cnt;
}

STATIC UINT32 inner_shell_get_cmd_key_by_tab(UINT32 *keys, CHAR *input)
{
    DLINK_NODE *iter = NULL;
    UINT32 key_index = 0;
    UINT32 *ptr = NULL;
    DLINK_FOREACH(iter, &(g_cmd_head.list)) {
        CMD_NODE *cmd = STRUCT_ENTRY(CMD_NODE, list, iter);
        if (strncmp((VOID *)input, cmd->cmd_name, strlen(input)) == 0) {
            ptr = &(keys[key_index++]);
            *ptr = (UINT32)&(cmd->cmd_name[0]);
        }
    }
    return key_index;
}

STATIC VOID inner_shell_uarrow_key_do()
{
    SHELL_PRINT("\r\n");
    inner_shell_clean_line();
    g_shell_history.history_cursor++;
    if (g_shell_history.history_cursor >= OSZ_CFG_SHELL_HISTORY_CMD_NUM) {
        g_shell_history.history_cursor = OSZ_CFG_SHELL_HISTORY_CMD_NUM;
    }
    if (g_shell_history.history_cmds[g_shell_history.history_cursor] == NULL) {
        g_shell_history.history_cursor -= 1; 
        SHELL_PRINT("\r%s$ %s", SHELL_NAME, g_shell_cb.buf);
        return;
    }
    if (g_shell_history.history_cursor >= OSZ_CFG_SHELL_HISTORY_CMD_NUM) {
        SHELL_PRINT("\r%s$ %s", SHELL_NAME, g_shell_cb.buf);
        return;
    }
    memset(g_shell_cb.buf, 0, sizeof(g_shell_cb.buf));
    UINT32 cmd_len = strlen(g_shell_history.history_cmds[g_shell_history.history_cursor]);
    memcpy((VOID *)g_shell_cb.buf, g_shell_history.history_cmds[g_shell_history.history_cursor], cmd_len);
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf + cmd_len;
    g_shell_cb.buf_cur_size = cmd_len;
    SHELL_PRINT("\r%s$ %s", SHELL_NAME, g_shell_cb.buf);
}

STATIC VOID inner_shell_darrow_key_do()
{
    SHELL_PRINT("\r");
    inner_shell_clean_line();
    if ((g_shell_history.history_cursor == SHELL_CMD_CURSOR_INVALID)) {
        SHELL_PRINT("\r%s$ ", SHELL_NAME);
        return;
    }
    g_shell_history.history_cursor--;
    if ((g_shell_history.history_cursor == SHELL_CMD_CURSOR_INVALID)) {
        memset(g_shell_cb.buf, 0, sizeof(g_shell_cb.buf));
        g_shell_cb.shell_buf_cursor = g_shell_cb.buf;
        g_shell_cb.buf_cur_size = 0;
        SHELL_PRINT("\r%s$ ", SHELL_NAME);
        return;
    }
    memset(g_shell_cb.buf, 0, sizeof(g_shell_cb.buf));
    UINT32 cmd_len = strlen(g_shell_history.history_cmds[g_shell_history.history_cursor]);
    memcpy((VOID *)g_shell_cb.buf, g_shell_history.history_cmds[g_shell_history.history_cursor], cmd_len);
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf + cmd_len;
    g_shell_cb.buf_cur_size = cmd_len;
    SHELL_PRINT("\r%s$ %s", SHELL_NAME, g_shell_cb.buf);
}

STATIC VOID inner_shell_larrow_key_do()
{
    if (g_shell_cb.shell_buf_cursor != g_shell_cb.buf) {
        g_shell_cb.shell_buf_cursor--;
    } else {
        CHAR c = (g_shell_cb.buf[0] == '\0') ? SHELL_SPECIAL_CHAR_SPACE : g_shell_cb.buf[0];
        SHELL_PRINT(" %c\b", c);
    }
}

STATIC VOID inner_shell_rarrow_key_do()
{
    if (g_shell_cb.shell_buf_cursor != (g_shell_cb.buf + g_shell_cb.buf_cur_size)) {
        g_shell_cb.shell_buf_cursor++;
    } else {
        SHELL_PRINT("\b");
    }
}

STATIC VOID inner_shell_del_key_do(VOID)
{
    SHELL_PRINT("\r");
    inner_shell_clean_line();
    SHELL_PRINT("\r%s$ ", SHELL_NAME);
    if (g_shell_cb.buf_cur_size == 0) {
        return;
    }
    if (g_shell_cb.shell_buf_cursor == g_shell_cb.buf) {
        SHELL_PRINT("%s", g_shell_cb.buf);
        SHELL_PRINT("\r%s$ ", SHELL_NAME);
        return;
    }
    CHAR *tmp = (CHAR *)osz_zalloc(g_shell_cb.buf_cur_size + 1);
    strncpy((VOID *)tmp, (VOID *)g_shell_cb.buf, g_shell_cb.buf_cur_size);
    UINT32 tmp_len = strlen(tmp);
    memset((VOID *)g_shell_cb.buf, 0, g_shell_cb.buf_cur_size);
    g_shell_cb.buf_cur_size = g_shell_cb.shell_buf_cursor - g_shell_cb.buf - 1;
    strncpy((VOID *)g_shell_cb.buf, (VOID *)tmp, g_shell_cb.buf_cur_size);
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf + g_shell_cb.buf_cur_size;
    if (g_shell_cb.buf_cur_size < (tmp_len - 1)) {
        strncpy((VOID *)g_shell_cb.shell_buf_cursor, (VOID *)(tmp + g_shell_cb.buf_cur_size + 1), tmp_len - g_shell_cb.buf_cur_size - 1);
        g_shell_cb.buf_cur_size = tmp_len - 1;
    }
    osz_free((VOID *)tmp);
    SHELL_PRINT("%s", g_shell_cb.buf);
    for (UINT32 i = 0; i < (g_shell_cb.buf_cur_size - (g_shell_cb.shell_buf_cursor - g_shell_cb.buf)); ++i) {
        SHELL_PRINT("\b");
    }
}

STATIC VOID inner_shell_tab_key_do(VOID)
{
    UINT32 count = inner_shell_get_cmd_key_count_by_tab();
    if (count == 0) {
        return;
    }
    UINT32 *keys = (UINT32 *)osz_zalloc(sizeof(UINT32) * count);
    count = inner_shell_get_cmd_key_by_tab(keys, g_shell_cb.buf);
    if (count == 1) {
        g_shell_cb.buf_cur_size = strlen((CHAR *)(keys[0]));
        memcpy((VOID *)g_shell_cb.buf, (VOID *)(CHAR *)(keys[0]), (size_t)g_shell_cb.buf_cur_size);
        g_shell_cb.shell_buf_cursor = g_shell_cb.buf + g_shell_cb.buf_cur_size;
        SHELL_PRINT("\r");
        inner_shell_clean_line();
        SHELL_PRINT("\r%s$ %s", SHELL_NAME, g_shell_cb.buf);
        return osz_free((VOID *)keys);
    }
    SHELL_PRINT("\n");
    for (UINT32 i = 0; i < count; ++i) {
        SHELL_PRINT("%s ", (CHAR *)(keys[i]));
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
    UINT32 ret = shell_read_fifo();
    if (ret != SHELL_FIFO_READ_VALID) {
        g_shell_cb.shell_state = SHELL_STATE_ERR;
        g_shell_cb.shell_err_code = SHELL_FIFO_READ_ERR;
        return;
    }
    if (inner_shell_char_is_special(g_shell_cb.shell_cur_char)) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        return;
    }
    BOOL cursor_has_shifted = inner_shell_buf_cursor_has_shifted();
    CHAR *tmp_buf = NULL;
    UINT32 gap_size = 0;
    if (cursor_has_shifted) {
        gap_size = g_shell_cb.buf_cur_size - (g_shell_cb.shell_buf_cursor - g_shell_cb.buf);
        tmp_buf = (CHAR *)osz_zalloc(gap_size+1);
        memcpy((VOID *)tmp_buf, (VOID *)g_shell_cb.shell_buf_cursor, gap_size);
    }
    *g_shell_cb.shell_buf_cursor = g_shell_cb.shell_cur_char;
    g_shell_cb.buf_cur_size++;
    g_shell_cb.shell_buf_cursor++;
    if (cursor_has_shifted && (tmp_buf != NULL)) {
        memcpy((VOID *)g_shell_cb.shell_buf_cursor, (VOID *)tmp_buf, gap_size);
        osz_free(tmp_buf);
        tmp_buf = NULL;
        inner_shell_reprint_with_cursor_shift(gap_size);
    }
    g_shell_history.history_max_cmd_len = (g_shell_history.history_max_cmd_len >= g_shell_cb.buf_cur_size) ? g_shell_history.history_max_cmd_len : g_shell_cb.buf_cur_size;
    g_shell_cb.shell_state = SHELL_STATE_NONE;
}

STATIC VOID inner_shell_deal_switch_phase()
{
    if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_DEL) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_del_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_HT) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_tab_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_CR) {
        g_shell_cb.shell_state = SHELL_STATE_PARSE;
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_ESC) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        UINT32 ret = shell_read_fifo();
        if (ret != SHELL_FIFO_READ_VALID) {
            g_shell_cb.shell_state = SHELL_STATE_ERR;
            g_shell_cb.shell_err_code = SHELL_FIFO_READ_ERR;
        }
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_LSB) {
        g_shell_cb.shell_state = SHELL_STATE_SWITCH;
        UINT32 ret = shell_read_fifo();
        if (ret != SHELL_FIFO_READ_VALID) {
            g_shell_cb.shell_state = SHELL_STATE_ERR;
            g_shell_cb.shell_err_code = SHELL_FIFO_READ_ERR;
        }
        return;
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_A) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_uarrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_B) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_darrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_C) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_rarrow_key_do();
    } else if (g_shell_cb.shell_cur_char == SHELL_SPECIAL_CHAR_D) {
        g_shell_cb.shell_state = SHELL_STATE_NONE;
        return inner_shell_larrow_key_do();
    }
    return;
}

STATIC VOID inner_shell_deal_parse_phase()
{
    CHAR *name[SHELL_CMD_KEY_LEN_MAX] = { 0 };
    CHAR **args = NULL;
    CHAR *buf_ptr = g_shell_cb.buf;
    CHAR *buf_end = g_shell_cb.buf + g_shell_cb.buf_cur_size;
    inner_shell_record_history_cmd();
    inner_shell_change_space_to_zero();
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
        if (strncmp(cmd->cmd_name, name, strlen(cmd->cmd_name)) == 0) {
            break;
        }
    }
    if (iter != &(g_cmd_head.list)) {
        if (cmd->max_argc != 0) {
            args = osz_zalloc(sizeof(CHAR *) * cmd->max_argc);
            for (;(buf_ptr < buf_end); buf_ptr += (strlen(buf_ptr) + 1)) {
                if (cmd->cur_argc <= cmd->max_argc) {
                    args[cmd->cur_argc] = buf_ptr;
                }
                cmd->cur_argc++;
            }
        }
        cmd->args = args;
    } else {
        cmd = NULL;
    }
    g_shell_cb.cur_cmd = cmd;
    g_shell_cb.shell_state = SHELL_STATE_EXC;
}

STATIC VOID inner_shell_deal_exc_phase()
{
    g_shell_cb.shell_state = SHELL_STATE_NONE;
    CMD_NODE *cmd = g_shell_cb.cur_cmd;
    if (cmd != NULL) {
        cmd->cmd_func(cmd->cur_argc, cmd->args);
    } else {
        SHELL_PRINT("%s: No such command.\n", g_shell_cb.buf);
    }
    
    memset(g_shell_cb.buf, 0, g_shell_cb.shell_capcity);
    g_shell_cb.buf_cur_size = 0;
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf;
    g_shell_history.history_cursor = SHELL_CMD_CURSOR_INVALID;
    if (cmd != NULL && cmd->args != NULL) {
        osz_free(cmd->args);
        cmd->args = NULL;
        cmd->cur_argc = 0;
    }
}

STATIC VOID inner_shell_deal_err_phase()
{
    memset(g_shell_cb.buf, 0, g_shell_cb.shell_capcity);
    g_shell_cb.buf_cur_size = 0;
    g_shell_cb.shell_err_code = '\0';
    g_shell_cb.shell_buf_cursor = g_shell_cb.buf;
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
    cmd->max_argc = params->argc;
    cmd->cmd_func = params->cmd_func;
    cmd->cmd_name = params->cmd_name;
    dlink_insert_tail(&(g_cmd_head.list), &(cmd->list));
}

VOID shell_loop(VOID)
{
    SHELL_PRINT("%s$ ", SHELL_NAME);
    while (TRUE) {
        switch (g_shell_cb.shell_state) {
            case SHELL_STATE_NONE:
                if (!fifo_is_empty(g_shell_cb.fifo)) {
                    g_shell_cb.shell_state = SHELL_STATE_GET;
                } else {
                    osz_msleep(20);
                }
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

VOID shell_write_fifo(CHAR c)
{
    fifo_write(g_shell_cb.fifo, (VOID *)(&c), 1, FIFO_WRITE_STOP);
}

UINT32 shell_read_fifo(VOID)
{
    CHAR c = 0;
    UINT32 cnt = fifo_read(g_shell_cb.fifo, &c, 1);
    g_shell_cb.shell_cur_char = c;
    return cnt;
}