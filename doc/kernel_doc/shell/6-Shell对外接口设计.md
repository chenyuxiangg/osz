## 对外接口

Shell模块提供以下对外接口，供其他模块调用以实现命令行交互功能。这些接口在`kernel/shell/include/shell.h`头文件中声明，是Shell模块与外部系统交互的桥梁。

### 命令注册接口

#### `VOID shell_register_cmd(CMD_PARAMS *params)`
- **作用**：注册动态命令到Shell系统中。该函数将命令参数添加到命令链表中，支持运行时动态添加命令，扩展系统功能。
- **限制**：
  - 命令名不能为空字符串，长度受`SHELL_CMD_KEY_LEN_MAX`（0x16）限制
  - 回调函数指针不能为NULL
  - 命令名在系统中必须唯一，重复注册可能导致不可预测行为
  - 最大参数数量`argc`应合理设置，过多可能导致内存浪费
- **使用流程**：
  1. 准备`CMD_PARAMS`结构体，包含命令名、回调函数和最大参数数量
  2. 调用`shell_register_cmd()`函数注册命令
  3. 函数内部使用`ASSERT()`检查参数指针有效性
  4. 调用`inner_shell_check_params()`验证参数合法性
  5. 分配`CMD_NODE`结构体内存并初始化
  6. 将命令节点插入命令链表尾部
  7. 注册成功后，命令即可在Shell中通过名称调用
- **典型应用场景**：
  - 系统初始化时注册核心命令
  - 模块加载时注册模块特定命令
  - 调试时临时添加测试命令

### Shell任务管理接口

#### `VOID shell_loop(VOID)`
- **作用**：Shell主循环函数，实现状态机的主循环逻辑。该函数作为独立任务运行，处理用户输入、命令解析和执行。
- **限制**：
  - 必须作为独立任务运行，依赖任务调度系统
  - 函数不会返回，一直运行直到系统关闭
  - 依赖全局状态变量`g_shell_cb.shell_state`，外部不应直接修改
- **使用流程**：
  1. 系统初始化时调用`inner_shell_task_create()`创建Shell任务
  2. Shell任务入口函数调用`shell_loop()`
  3. 函数进入无限循环，根据当前状态执行相应操作：
     - `SHELL_STATE_NONE`：检查FIFO队列，非空则切换到GET状态
     - `SHELL_STATE_GET`：调用`inner_shell_deal_get_phase()`处理输入字符
     - `SHELL_STATE_SWITCH`：调用`inner_shell_deal_switch_phase()`处理特殊字符
     - `SHELL_STATE_PARSE`：调用`inner_shell_deal_parse_phase()`解析命令
     - `SHELL_STATE_EXC`：调用`inner_shell_deal_exc_phase()`执行命令
     - `SHELL_STATE_ERR`：调用`inner_shell_deal_err_phase()`处理错误
  4. 每个循环周期主动睡眠20ms，避免独占CPU资源
- **典型应用场景**：
  - Shell任务的主函数，由任务调度系统调用
  - 不应由外部模块直接调用

### 输入输出接口

#### `VOID shell_write_fifo(CHAR c)`
- **作用**：向Shell的FIFO输入队列写入字符，实现异步输入处理。通常由UART中断服务程序调用，将接收到的字符传递给Shell处理。
- **限制**：
  - FIFO队列大小为`SHELL_FIFO_MAX_SIZE`（0x40），队列满时新字符会被丢弃
  - 需要确保Shell模块已正确初始化，FIFO队列已创建
  - 仅支持单字符写入，不支持批量写入
- **使用流程**：
  1. UART中断服务程序接收到字符
  2. 调用`shell_write_fifo()`将字符写入FIFO队列
  3. 函数内部获取Shell控制块中的FIFO指针
  4. 调用`fifo_write()`以`FIFO_WRITE_STOP`模式写入字符
  5. 如果队列已满，写入操作停止，字符被丢弃
  6. Shell任务在`SHELL_STATE_NONE`状态检测到FIFO非空，切换到`SHELL_STATE_GET`状态处理字符
- **典型应用场景**：
  - UART中断服务程序处理用户输入
  - 其他输入设备（如键盘）的中断处理
  - 测试时模拟用户输入

#### `UINT32 shell_read_fifo(VOID)`
- **作用**：从Shell的FIFO输入队列读取字符，供Shell状态机处理用户输入。该函数在`SHELL_STATE_GET`状态被调用。
- **限制**：
  - 仅由Shell模块内部调用，外部模块不应直接使用
  - FIFO队列为空时返回0，表示没有可读取的字符
  - 每次只读取一个字符
- **使用流程**：
  1. Shell处于`SHELL_STATE_GET`状态时调用
  2. 获取Shell控制块中的FIFO指针
  3. 调用`fifo_read()`读取一个字符
  4. 将读取的字符存储到`g_shell_cb.shell_cur_char`
  5. 返回读取的字符数量（1表示成功，0表示失败）
  6. 如果读取成功，字符被传递给状态机进一步处理
- **典型应用场景**：
  - Shell状态机处理用户输入字符
  - 仅在Shell模块内部使用

### 状态管理接口

#### `SHELL_STATE shell_get_state(VOID)`
- **作用**：获取Shell当前运行状态，供外部模块查询Shell状态，实现状态感知和协调。
- **限制**：
  - 仅返回状态枚举值，不提供状态语义解释
  - 状态可能随时变化，获取的状态是调用时刻的快照
  - 外部模块不应依赖状态进行关键决策
- **使用流程**：
  1. 外部模块需要了解Shell状态时调用
  2. 函数返回`g_shell_cb.shell_state`的当前值
  3. 调用者根据返回的状态值进行相应处理
  4. 状态值为`SHELL_STATE`枚举类型，包括：`NONE`、`GET`、`SWITCH`、`PARSE`、`EXC`、`ERR`
- **典型应用场景**：
  - 监控系统查询Shell运行状态
  - 调试工具显示Shell当前状态
  - 其他模块根据Shell状态协调操作

#### `VOID shell_set_state(SHELL_STATE state)`
- **作用**：设置Shell运行状态，允许外部模块控制Shell状态转换。主要用于调试和特殊控制场景。
- **限制**：
  - 需要谨慎使用，不当的状态设置可能破坏Shell正常流程
  - 设置的状态值必须是有效的`SHELL_STATE`枚举值
  - 不推荐在生产代码中随意修改Shell状态
  - 可能干扰正常的用户交互流程
- **使用流程**：
  1. 外部模块需要控制Shell状态时调用
  2. 提供有效的`SHELL_STATE`枚举值作为参数
  3. 函数将参数值赋值给`g_shell_cb.shell_state`
  4. Shell状态机在下一次循环中基于新状态执行相应操作
- **典型应用场景**：
  - 调试时强制设置Shell状态
  - 测试时模拟特定状态
  - 系统恢复时重置Shell状态

### 宏定义接口

#### `REGISTER_STATIC_CMD(cmd_, func_, max_argc_)`
- **作用**：静态命令注册宏，用于在编译期注册命令到Shell系统。通过链接器段机制收集所有静态命令，在系统初始化时自动注册。
- **限制**：
  - 仅能在编译期使用，不支持运行时动态注册
  - 命令名必须是字符串字面量
  - 回调函数必须是编译期可确定的函数指针
  - 最大参数数量必须是编译期常量
- **使用流程**：
  1. 在源文件中使用宏定义静态命令：
     ```c
     REGISTER_STATIC_CMD("help", help_cmd_func, 1);
     ```
  2. 宏展开为`CMD_PARAMS`结构体定义，并放入`.osz_cmd`链接器段
  3. 系统初始化时，`inner_shell_register_system_cmd()`遍历`.osz_cmd`段
  4. 对每个静态命令调用`shell_register_cmd()`注册
  5. 注册完成后，命令即可在Shell中使用
- **典型应用场景**：
  - 系统核心命令注册（如help、version等）
  - 常用工具命令注册
  - 编译期确定的固定命令集