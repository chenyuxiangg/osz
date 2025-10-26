## 概述

该文档仅适用于osz代码仓生成ST测试用例代码生成，描述生成测试用例时的要求与约束。

## 测试用例要求

1. 测试用例代码仅可以写在`TODO`注释行之后，且必须保留文件中的其他内容不变；
2. 每条测试用例必须符合测试用例注释中Preconditions、Steps、Expected的要求；
3. 每条测试用例生成后必须由用户批准；
5. 若某接口中使用了上下文中无法找到的函数、变量、宏，优先在整个项目的所有.h中搜索，搜索到后将需要的头文件include到测试文件中，且只需要include文件名；
6. 测试用例所需的内存相关接口，优先参考`kernel/base/mem`目录下的文件；
7. 测试用例所需的任务相关接口，优先参考`kernel/base/task`目录下的文件；
8. 测试用例所需的调度相关的接口，优先参考`kernel/base/schedule`目录下的文件，且必须排除`kernel/base/schedule/coordination`目录；
9. 测试用例所需的ipc相关接口，优先参考`kernel/base/ipc`目录下的文件；
10. 测试用例所需的体系结构相关的接口，优先参考`arch`目录下对应体结构目录；
11. 测试用例一定不能使用标准库函数；
12. 当符号无法在整个项目中搜索到时，必须询问用户下一步操作；
13. 测试用例需要检查结果与预期是否符合时，只能使用`VERIFY()`宏，其中参数为一个返回值为bool类型的表达式，比如：`VERIFY(sem == NULL)`;
14. 当测试用例需要实现新函数时，需要将测试需要的新函数实现在helper文件中，例如当前正在往`test_sem.c`文件中写测试用例，则应该将测试需要的新函数实现在`test_sem_helper.c`文件中，新函数需要声明在`test_sem_helper.h`中；
15. helper的.c文件必须与测试文件处于同级目录，例如`test_st_sem_g1.c`位于`test_suite/suite/src/sem/st`路径下，则`test_sem_helper.c`必须也在`test_suite/suite/src/sem/st/`路径下；
16. helper的.h文件必须在suite目录下的`include/st`目录下，例如`test_st_sem_g1.c`位于`test_suite/suite/src/sem/st`路径下，则`test_sem_helper.h`必须在`test_suite/suite/include/st`路径下；
17. 当文件不存在时，需要创建相应文件；
18. 新增加的函数必须按顺序追加到文件中；

## 代码规范

1. 代码必须保证可读性强；
2. 函数、变量及类型必须遵循用户自定义风格，例如：`uint32_t osz_sem_init(uint8_t *name, uint8_t name_size, uint32_t val, osz_sem_t **outter_obj)`；
3. 当需要使用到基础类型时，优先参考`kernel/comm/include/ztype.h`文件中的定义；