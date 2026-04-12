## 数据结构

### CMD_HISTORY
- **结构体字段解释**：
	- `CHAR *history_cmds[OSZ_CFG_SHELL_HISTORY_CMD_NUM]`：历史命令指针数组，存储历史命令字符串。数组大小由配置项`OSZ_CFG_SHELL_HISTORY_CMD_NUM`决定，默认值为10。
    - `UINT32 history_next_cmd_idx : 8`：下一条命令记录的索引。取值范围：0-255。
    - `UINT32 history_cursor : 8`：当前浏览的历史命令索引。取值范围：0-254为有效索引，255（`SHELL_CMD_CURSOR_INVALID`）表示无效光标。
    - `UINT32 history_has_cmd : 1`：是否有历史命令标志。0表示没有历史命令，1表示有历史命令。
    - `UINT32 history_max_cmd_len : 7`：历史命令中的最大长度。取值范围：0-127，用于清屏时计算覆盖长度。
    - `UINT32 reserv : 8`：保留位，目前未使用。
 
### CMD_NODE
- **结构体字段解释**：
    - `DLINK_NODE list`：双向链表节点，用于连接命令链表中的各个命令节点。
    - `CHAR *cmd_name`：命令名称字符串指针，指向命令名字符串。
    - `CMD_CALLBACK_FUNC cmd_func`：命令回调函数指针，当命令被执行时调用此函数。
    - `CHAR **args`：命令参数数组指针，动态分配内存存储解析后的参数。
    - `UINT16 max_argc`：命令支持的最大参数数量，由注册时指定。
    - `UINT16 cur_argc`：当前解析到的参数数量，在执行命令时使用。
 
### SHELL_CB
- **结构体字段解释**：
    - `CHAR buf[SHELL_BUFFER_MAX_NUM]`：输入缓冲区，用于存储用户输入的命令字符。大小为`0x400`（1024）字节。
    - `FIFO *fifo`：FIFO队列指针，用于存储从UART接收的字符，实现异步输入处理。
    - `UINT32 buf_cur_size : 10`：当前缓冲区中有效字符的数量。取值范围：0-1023（2^10-1）。
    - `UINT32 shell_capcity : 11`：缓冲区容量，固定为`SHELL_BUFFER_MAX_NUM - 1 = 1023`。
    - 联合体（共用8位）：
      - `UINT32 shell_err_code : 8`：错误代码，当Shell发生错误时存储错误码。取值范围：0-255。
      - `UINT32 shell_cur_char : 8`：当前从FIFO读取的字符。取值范围：0-255（ASCII字符范围）。
    - `UINT32 shell_state : 3`：Shell状态机当前状态。取值范围：0-7，对应`SHELL_STATE`枚举值。
    - `CHAR *shell_buf_cursor`：缓冲区光标指针，指向当前编辑位置，支持光标移动和插入删除操作。
    - `CMD_NODE *cur_cmd`：当前正在执行的命令节点指针，指向匹配到的命令节点。

### SHELL_STATE
  ```c
  typedef enum {
      SHELL_STATE_NONE = 0,   // 空闲状态，等待输入
      SHELL_STATE_GET,        // 获取字符状态，处理输入
      SHELL_STATE_SWITCH,     // 切换状态，处理特殊字符
      SHELL_STATE_PARSE,      // 解析状态，解析命令和参数
      SHELL_STATE_EXC,        // 执行状态，执行命令
      SHELL_STATE_ERR,        // 错误状态，处理错误
  } SHELL_STATE;
  ```

### CMD_PARAMS
注册命令时传入的参数类型。
- **结构体字段解释**：
	- `CHAR *cmd_name`: 字符串指针，不能为空，指向命令名字符串。
	- `CMD_CALLBACK_FUNC cmd_func`: 命令回调函数指针，不能为空，指向命令回调函数。
	- `UINT32 argc`: 命令回调参数个数，各参数的具体含义由命令回调定义。

其中`CMD_CALLBACK_FUNC`类型定义如下：
```c
  typedef void (*CMD_CALLBACK_FUNC)(UINT32 argc, CHAR *argv[]);
```

## 控制变量
### 外部链接器符号
- **`extern UINT32 __cmd_start;`** 和 **`extern UINT32 __cmd_end;`**
  - **作用**：链接器定义的符号，标记静态命令段(`.osz_cmd`)的起始和结束地址。这些符号由链接脚本生成，用于定位所有使用`REGISTER_STATIC_CMD`宏静态注册的命令。
  - **取值范围**：由链接器根据实际静态命令的数量和大小决定，指向内存中CMD_PARAMS结构体数组的边界。

### 全局变量
- **`UINT32 g_cmd_start = (UINT32)&__cmd_start;`**
  - **作用**：存储静态命令段的起始地址，用于在初始化时遍历所有静态注册的命令。
  - **取值范围**：有效的内存地址，指向CMD_PARAMS结构体数组的起始位置。
  - **类型**：`UINT32`（32位无符号整数）

- **`UINT32 g_cmd_end = (UINT32)&__cmd_end;`**
  - **作用**：存储静态命令段的结束地址，与`g_cmd_start`配合使用，确定静态命令的遍历范围。
  - **取值范围**：有效的内存地址，指向CMD_PARAMS结构体数组的结束位置。
  - **类型**：`UINT32`（32位无符号整数）

### 静态全局变量
- **`STATIC CMD_NODE g_cmd_head;`**
  - **作用**：命令链表的头节点，用于管理所有注册的命令（包括静态注册和动态注册）。所有命令通过双向链表连接，便于查找和执行。
  - **类型**：`CMD_NODE`结构体

- **`STATIC SHELL_CB g_shell_cb;`**
  - **作用**：Shell控制块，包含Shell运行时的所有状态、数据和缓冲区，是Shell模块的核心数据结构。
  - **类型**：`SHELL_CB`结构体

- **`STATIC CMD_HISTORY g_shell_history;`**
  - **作用**：命令历史记录管理，支持上下箭头浏览历史命令，提高用户交互体验。
  - **类型**：`CMD_HISTORY`结构体