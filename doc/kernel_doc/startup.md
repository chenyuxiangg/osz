## OSZ启动流程

本文将介绍OSZ从系统上电开始到`main`函数第一条指令运行中间的整个过程。

![系统启动图示]()

OSZ中启动主要分为两大部分：C运行环境初始化、内核资源初始化

### C运行环境初始化

裸机环境下是不能直接跑C代码的，因为C语言的运行至少需要保证**栈指针**已经初始化完成。所以运行C代码前必然需要先进行栈的初始化。

> *Tips:*
> *1. C语言开发者不会直接感知到栈指针，压栈和出栈的操作有编译器处理；*

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

初始化等级*l1*为最高等级，将被优先执行。相同等级的多个初始化函数的执行顺序将在编译时确定，因此相同优先级的初始化函数中不应该存在依赖关系。

当前内核中初始化函数及其优先级如下：

 l0 | init_osz_interrupt_init   |                    |                           |                             |                     |
| -- | ------------------------- | ------------------ | ------------------------- | --------------------------- | ------------------- |
| l1 | init_osz_memory_init      | init_drv_uart_init | init_inner_pri_queue_init | init_inner_task_module_init | init_drv_timer_init |
| l2 | init_os_preemp_sched_init |                    |                           |                             |                     |
| l3 | init_inner_tick_init      |                    |                           |                             |                     |
| l4 | init_inner_shell_init     |                    |                           |                             |                     |

内核初始化完成后，将会由`os_init`函数跳转到`main`，业务将从这里开始。