## OSZ启动流程

本文将介绍OSZ从系统上电开始到`main`函数第一条指令运行中间的整个过程。

![系统启动图示](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/images/osz_init_flow.png)

OSZ中启动主要分为两大部分：C运行环境初始化、内核资源初始化

### C运行环境初始化

裸机环境下是不能直接跑C代码的，因为C语言的运行至少需要保证**栈指针**已经初始化完成。所以运行C代码前必然需要先进行栈的初始化。

> *Tips:*
> *1. C语言开发者不会直接感知到栈指针，压栈和出栈的操作有编译器处理；*

#### RISCV栈增长方向

![RISCV栈增长方向](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/images/riscv_stack_grow_dir.png)

RISCV中栈是从高地址向低地址增长，即**栈顶位于高地址，压栈时先将sp减小后再保存数据。**

### 内核资源初始化

OSZ中内核资源初始化是自动进行的，实现原理是**在main函数执行前调用资源的初始化接口(参见kernel/base/init/src/init.c)**。初始化执行的优先级通过链接脚本指定，即：

```txt
.init : {
		__init_start = .;
		__init_l0_start = .;
		KEEP(*(.init.init_l0))
		__init_l0_end = .;
		__init_l1_start = .;
		KEEP(*(.init.init_l1))
		__init_l1_end = .;
		__init_l2_start = .;
		KEEP(*(.init.init_l2))
		__init_l2_end = .;
		__init_l3_start = .;
		KEEP(*(.init.init_l3))
		__init_l3_end = .;
		__init_l4_start = .;
		KEEP(*(.init.init_l4))
		__init_l4_end = .;
		__init_end = .;
	} > INIT :init
```

初始化等级*l1*为最高等级，将被优先执行。相同等级的多个初始化函数的执行顺序将在编译时确定，因此**相同优先级的初始化函数**中不应该存在依赖关系。

当前内核中初始化函数及其优先级如下：

 l0 | init_osz_interrupt_init   |                    |                           |                             |                     |
| -- | ------------------------- | ------------------ | ------------------------- | --------------------------- | ------------------- |
| l1 | init_osz_memory_init      | init_drv_uart_init | init_inner_pri_queue_init | init_inner_task_module_init | init_drv_timer_init |
| l2 | init_os_preemp_sched_init |                    |                           |                             |                     |
| l3 | init_inner_tick_init      |                    |                           |                             |                     |
| l4 | init_inner_shell_init     |                    |                           |                             |                     |

内核初始化完成后，将会由`os_init`函数跳转到`main`，`main`函数需要由各个项目自行编写，其中业建议包含业务的必要初始化，建议的分层为：

1. 芯片初始化（chip_init）：用于初始化业务所用到的驱动，比如pmp、watchdog、uart等；
2. 系统服务初始化（servers_init）：用于初始化业务所用到的服务，比如log；
3. 业务线程初始化（threads_init）：用于初始化业务的所有任务；

无论`main`函数中包含哪些自定义的流程，**务必在函数末尾调用`first_schedule`**启动调度，此时系统才能正常运转。