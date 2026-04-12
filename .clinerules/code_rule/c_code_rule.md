# MISRA-C 编码规范

## 概述

MISRA-C（Motor Industry Software Reliability Association C）是汽车行业广泛采用的C语言编码规范，用于规范项目中C语言编码，旨在提高嵌入式系统代码的安全性、可靠性和可移植性。本规范基于MISRA-C:2012标准，选取了最关键的规则供项目参考。

与《c_code_rule_ext.md》配合使用。

## 核心规则

### 1. 强制规则（Required）

#### 规则 1.1：所有代码必须符合ISO/IEC 9899:1990标准
- **要求**：使用标准C语言特性，避免编译器扩展
- **示例**：
  ```c
  // 正确：使用标准C
  int32_t value = 0;
  
  // 错误：使用编译器扩展
  int value = 0 __attribute__((aligned(16)));
  ```

#### 规则 2.1：禁止使用未声明的标识符
- **要求**：所有标识符必须先声明后使用
- **示例**：
  ```c
  // 正确：先声明后使用
  extern int32_t global_var;
  
  void func(void) {
      int32_t local_var = global_var + 1;
  }
  
  // 错误：使用未声明的标识符
  void func(void) {
      value = 10;  // 'value'未声明
  }
  ```

#### 规则 5.1：外部标识符不得重复
- **要求**：不同作用域中的标识符不应使用相同名称
- **示例**：
  ```c
  // 正确：使用不同的名称
  static int32_t local_counter;
  extern int32_t global_counter;
  
  // 错误：名称冲突
  static int32_t counter;
  extern int32_t counter;  // 与静态变量同名
  ```

### 2. 数据类型规则

#### 规则 10.1：禁止隐式类型转换
- **要求**：避免可能导致数据丢失的隐式类型转换
- **示例**：
  ```c
  // 正确：显式类型转换
  uint16_t a = 100;
  uint32_t b = (uint32_t)a + 50;
  
  // 错误：隐式类型转换
  uint16_t a = 100;
  uint32_t b = a + 50;  // 隐式转换，可能丢失精度
  ```

#### 规则 10.3：禁止有符号和无符号整型之间的隐式转换
- **要求**：有符号和无符号类型之间的转换必须显式进行
- **示例**：
  ```c
  // 正确：显式转换
  int32_t signed_val = -10;
  uint32_t unsigned_val = (uint32_t)signed_val;
  
  // 错误：隐式转换
  int32_t signed_val = -10;
  uint32_t unsigned_val = signed_val;  // 隐式转换，行为未定义
  ```

### 3. 控制流规则

#### 规则 14.1：禁止使用goto语句
- **要求**：避免使用goto语句，使用结构化控制流
- **示例**：
  ```c
  // 正确：使用结构化控制流
  void process_data(void) {
      if (condition1) {
          // 处理条件1
          return;
      }
      
      if (condition2) {
          // 处理条件2
          return;
      }
      
      // 默认处理
  }
  
  // 错误：使用goto
  void process_data(void) {
      if (error) {
          goto cleanup;
      }
      // ...
  cleanup:
      // 清理代码
  }
  ```

#### 规则 14.4：switch语句必须有default分支
- **要求**：所有switch语句必须包含default分支
- **示例**：
  ```c
  // 正确：包含default分支
  switch (state) {
      case STATE_IDLE:
          handle_idle();
          break;
      case STATE_ACTIVE:
          handle_active();
          break;
      default:
          handle_unknown();
          break;
  }
  
  // 错误：缺少default分支
  switch (state) {
      case STATE_IDLE:
          handle_idle();
          break;
      case STATE_ACTIVE:
          handle_active();
          break;
      // 缺少default分支
  }
  ```

#### 规则 15.1：禁止使用空循环体
- **要求**：循环体必须包含至少一条语句
- **示例**：
  ```c
  // 正确：使用明确的空语句
  while (!ready) {
      // 等待就绪
  }
  
  // 错误：空循环体
  while (!ready);  // 空循环体，容易引起误解
  ```

### 4. 内存管理规则

#### 规则 17.1：禁止指针算术运算
- **要求**：避免使用指针算术，使用数组索引
- **示例**：
  ```c
  // 正确：使用数组索引
  uint8_t buffer[100];
  for (uint32_t i = 0; i < 100; i++) {
      buffer[i] = 0;
  }
  
  // 错误：使用指针算术
  uint8_t buffer[100];
  uint8_t *ptr = buffer;
  for (uint32_t i = 0; i < 100; i++) {
      *ptr++ = 0;
  }
  ```

#### 规则 17.4：禁止使用指针类型转换
- **要求**：避免在不同类型的指针之间进行转换
- **示例**：
  ```c
  // 正确：使用相同类型的指针
  uint32_t *data_ptr = (uint32_t *)buffer;
  
  // 错误：不同类型指针转换
  uint8_t *byte_ptr = (uint8_t *)&data;  // 可能违反对齐要求
  ```

### 5. 预处理规则

#### 规则 20.1：禁止使用#include指令包含.c文件
- **要求**：只包含头文件，不包含源文件
- **示例**：
  ```c
  // 正确：包含头文件
  #include "module.h"
  
  // 错误：包含源文件
  #include "module.c"  // 绝对禁止
  ```

#### 规则 20.4：宏定义必须使用括号
- **要求**：宏定义中的参数和整个表达式必须用括号括起来
- **示例**：
  ```c
  // 正确：使用括号
  #define MAX(a, b) (((a) > (b)) ? (a) : (b))
  #define BUFFER_SIZE (1024)
  
  // 错误：缺少括号
  #define MAX(a, b) a > b ? a : b  // 可能产生错误结果
  #define BUFFER_SIZE 1024 * 2     // 可能产生错误结果
  ```

### 6. 函数规则

#### 规则 16.1：函数必须有明确的返回类型
- **要求**：所有函数必须声明返回类型，void函数除外
- **示例**：
  ```c
  // 正确：明确的返回类型
  int32_t calculate_sum(int32_t a, int32_t b) {
      return a + b;
  }
  
  void initialize_system(void) {
      // 初始化代码
  }
  
  // 错误：隐式返回类型
  calculate_sum(int32_t a, int32_t b) {  // 默认为int
      return a + b;
  }
  ```

#### 规则 16.6：函数参数数量不得超过8个
- **要求**：限制函数参数数量，提高可读性
- **示例**：
  ```c
  // 正确：参数数量合理
  void configure_device(uint32_t addr, uint32_t mode, uint32_t timeout) {
      // 配置代码
  }
  
  // 错误：参数过多
  void configure_all(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4,
                     uint32_t p5, uint32_t p6, uint32_t p7, uint32_t p8,
                     uint32_t p9) {  // 超过8个参数
      // 配置代码
  }
  ```

### 7. 错误处理规则

#### 规则 15.2：所有非void函数必须有返回值
- **要求**：确保函数在所有执行路径上都有返回值
- **示例**：
  ```c
  // 正确：所有路径都有返回值
  int32_t safe_divide(int32_t a, int32_t b) {
      if (b == 0) {
          return 0;  // 错误情况返回默认值
      }
      return a / b;
  }
  
  // 错误：可能没有返回值
  int32_t unsafe_divide(int32_t a, int32_t b) {
      if (b != 0) {
          return a / b;
      }
      // 错误情况没有返回值
  }
  ```

## 实施建议

1. **代码审查**：在代码审查中检查MISRA-C规则遵守情况
2. **静态分析工具**：使用支持MISRA-C的静态分析工具（如PC-lint、Coverity）
3. **编码标准检查表**：创建检查表确保关键规则得到遵守
4. **培训**：对开发人员进行MISRA-C规则培训

## 例外情况

在某些情况下，可能需要违反MISRA-C规则。此时必须：
1. 记录违反的规则和原因
2. 获得项目负责人的批准
3. 在代码中添加注释说明

## 参考资料

1. MISRA-C:2012 Guidelines for the use of the C language in critical systems
2. ISO/IEC 9899:1999 Programming languages - C
3. 项目特定编码规范

---
*最后更新：2026年4月11日*
*版本：1.0*
