# 项目C语言扩展编码规范

## 概述

该规范用于补充**c_code_rule.md**中未定义的规则。适用于当前项目。

## 核心规则

### 1. 强制规则（Required）

#### 规则 1.1：禁止代码行中保留冗余空格/空行
- **要求**：所有代码行中的空格/空行仅用于突出显示，冗余空格/空行必须删除
- **示例**：
```c
// 正确：空格/空行用于突出显示关键信息，保证可读性
if (a == 0) {
    c = b > a ? 0 : 1;
}
do_something();

// 错误：多余的换行操作
if (a == 0) {
    c = b > a ? 
        0 : 1;
}
do_something();
```

#### 规则 1.2：禁止使用tab缩进代码
- **要求**：强制代码缩进为4个空格
- **示例**：
```c
// 正确：4个空格缩进
while (a) {
    b = 1;
}
do_something();

// 错误：非4个空格缩进
if (a == 0) {
  b = 1;
}
do_something();
```

#### 规则 1.3：默认编码风格与代码上下文保持一致
- **要求**：未作要求的编码风格必须与代码上下文保持一致

#### 规则 1.4：禁止保留多余注释
- **要求**：注释仅用于困难逻辑解释或特殊场景，尽可能使用变量名、函数名、宏名自注释
- **示例**：
```c
// 正确：特殊场景注释，栈地址需要16bytes对齐
STATIC void_t inner_new_task_fill_magic_for_stack(osz_task_params_t *params)
{
    // must be keep stack_size align to 16
    for (uint32_t i = 0; i < params->stack_size/sizeof(uint32_t); ++i) {
        ((uint32_t *)params->stack_base)[i] = TASK_STACK_INIT_MAGIC;
    }
}

// 错误：所有注释都是多余
if (condition) {
    // 清空g_shell_cb.buf并重置当前行显示
    inner_shell_reset_line();
    inner_shell_reset_shellcb_buf();
} else {
    // 拷贝g_shell_history.history_cmds[g_shell_history.history_cursor]到g_shell_cb.buf
    inner_shell_get_history_cmd();
    // 将g_shell_cb.buf_cur_size赋值为g_shell_cb.buf的长度
    // 注意：inner_shell_get_history_cmd() 已经设置了 buf_cur_size
    // 重置g_shell_cb.shell_buf_cursor为g_shell_cb.buf
    // 注意：inner_shell_get_history_cmd() 已经设置了 shell_buf_cursor
    // 刷新显示，将g_shell_cb.buf内存显示出来
    inner_shell_reset_line();
    SHELL_PRINT("%s", g_shell_cb.buf);
}
```

#### 规则 1.5：禁止保留注释代码
- **要求**：如果代码被注释，说明也可以删除，不需要保留
- **示例**：
```c
// 错误：g_shell_history.history_cursor--;应该删除
if (g_shell_history.history_cursor == SHELL_CMD_CURSOR_INVALID) {
    UINT32 idx = (g_shell_history.history_next_cmd_idx == 0) ? (OSZ_CFG_SHELL_HISTORY_CMD_NUM - 1) : (g_shell_history.history_next_cmd_idx - 1);
    g_shell_history.history_cursor = idx;
} else {
    // g_shell_history.history_cursor--;
    g_shell_history.history_cursor = (g_shell_history.history_cursor == 0) ? (OSZ_CFG_SHELL_HISTORY_CMD_NUM - 1) : (g_shell_history.history_cursor - 1);
}
```