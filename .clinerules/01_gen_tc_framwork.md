## 概述

该文档仅适用于osz代码仓生成UT测试用例框架生成，描述生成测试用例时的要求与约束。

## 要求

1. 生成UT测试用例前，必须进行方案分析，并得到用户批准；
2. 每个模块都有独立的测试用例目录，且模块测试用例目录归档在`test_suite/suite/src/`路径下，例如sem模块，测试用例目录为`test_suite/suite/src/sem`；
3. 测试用例文件必须归档在对应的模块目录，比如sem模块的`test_sem_g1.c`测试文件，归档路径必须为`test_suite/suite/src/sem/test_sem_g1.c`；
4. 生成的测试用例文件格式必须参考`test_suite/suite/src/sem/test_sem_g1.c`文件;
5. 测试用例文件名必须以`test_`开头；
6. 测试用例文件必须以如下信息开头：
```c
/**
 * @file test_event.c
 * @brief Unit test cases for event module
 * @version 1.0
 * @date 2025
 * @author zyzs
 * 
 * @copyright Copyright (c) 2025 zyzs
 * 
 * @test
 * @brief Test suite for event functionality
 * 
 * Coverage targets:
 * - Statement coverage: 100%
 * - Branch coverage: 100%  
 * - Function coverage: 100%
 */
```
1. 测试用例文件必须有`void_t setup(void_t)`和`void_t teardown(void_t)`两个函数的留空实现；
2. 测试用例组必须用`TEST_GROUP()`宏定义，参数为测试组名，例如：
```c
TEST_GROUP("Semaphore Initialization Tests")
{
    ...
}
```
1. 测试用例必须用`TEST()`宏定义，参数为测试用例名，例如：
```c
TEST("Test_1_1: Normal Static Semaphore Initialization") {
    ...
}
```
1. 每条测试用例都应该包含：用例描述、前置条件、测试步骤以及预期结果，且这些信息应该以如下格式放置：
```c
TEST("") {
        // Preconditions: System initialized, static semaphore pool has available objects
        // Steps:
        //   1. Call osz_sem_init function with valid name, name length and initial count 5
        //   2. Check the returned output object pointer
        // Expected:
        //   - Function returns OS_OK
        //   - Output semaphore object pointer is not NULL
        //   - Semaphore count value is 5
        //   - Semaphore object marked as used state
        
        // TODO: Implement test code here
        
    }
```
1. 测试用例与测试组的代码组织必须遵循测试用例定义在测试组中，一个测试组可以包含多个测试用例，同一个测试用例不能放置在不同的测试组，例如：
```c
TEST_GROUP("Semaphore Initialization Tests")
{
    TEST("Test_1_1: Normal Static Semaphore Initialization") {
        ...
    }

    TEST("Test_1_2: Semaphore Initialization with Name") {
        ...
    }
    ...
}
```
1.  每个测试文件中有且仅有一个测试组；
2.  测试文件文件名应该包含组号，例如sem模块的第一组测试用例对应的测试文件名为`test_sem_g1.c`;