# OSZ

[![GitHub](https://img.shields.io/github/license/chenyuxiangg/osz)](https://github.com/chenyuxiangg/osz/blob/main/LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/chenyuxiangg/osz)](https://github.com/chenyuxiangg/osz/stargazers)
[![GitHub issues](https://img.shields.io/github/issues/chenyuxiangg/osz)](https://github.com/chenyuxiangg/osz/issues)
[![GitHub last commit](https://img.shields.io/github/last-commit/chenyuxiangg/osz)](https://github.com/chenyuxiangg/osz/commits/main)

**OSZ** 是一个轻量级实时操作系统（RTOS），专为资源受限的嵌入式系统设计，特别支持RISC-V架构。取名为"Z"的操作系统，旨在提供简洁、高效、可扩展的实时操作系统解决方案。

## ✨ 特性

### 内核特性
- **实时多任务支持**：基于优先级的抢占式调度
- **中断框架支持**：灵活的中断管理和处理机制
- **内存管理支持**：高效的内存分配和管理
- **IPC支持**：进程间通信机制（当前支持event）
- **抢占调度支持**：确保高优先级任务及时响应

### 硬件支持
- **RISC-V架构**：全面支持RISC-V指令集
- **QEMU RISC-V Virt单板支持**：便于仿真和测试
- **ns16550 UART驱动支持**：串口通信
- **mtime支持**：硬件定时器

### 开发工具
- **Shell命令行支持**：丰富的命令行界面
  - 支持上、下、左、右键导航
  - 支持tab补全/联想命令
  - 支持任意位置del字符
- **双构建系统支持**：CMake和Makefile双构建系统
- **交叉编译工具链集成**：开箱即用的RISC-V工具链
- **模块化编译框架**：灵活的组件配置和构建系统

### 第三方库集成
- **[printf](https://github.com/mpaland/printf)**：嵌入式中最好的printf实现，已源码集成
- 有且仅有一个外部依赖，保持系统精简

## 🚀 快速开始

### 环境准备

1. **克隆仓库**
   ```bash
   git clone https://github.com/chenyuxiangg/osz.git
   cd osz
   ```

2. **解压工具链**
   ```bash
   tar -xzf tools/riscv-unknown-elf.tar.gz -C tools/
   ```

### 使用CMake构建（推荐）

OSZ现在支持现代化的CMake构建系统：

```bash
# 进入构建目录
cd construct

# 查看可用的构建目标
./build_cmake.sh --help

# 构建所有目标
./build_cmake.sh

# 构建并运行QEMU仿真
./build_cmake.sh target_qemu_run

# 构建调试版本
./build_cmake.sh target_qemu_debug

# 清理构建产物
./build_cmake.sh clean
```

### 使用Makefile构建（传统方式）

传统的Makefile构建系统仍然可用：

```bash
cd construct
make            # 构建所有目标
make run        # 构建并运行QEMU
make debug      # 构建调试版本
make clean      # 清理构建产物
```

### 使用QEMU仿真

OSZ支持在QEMU中运行，无需物理硬件：

```bash
cd construct
./build_cmake.sh target_qemu_run    # 使用CMake构建并运行
# 或
make run                           # 使用Makefile构建并运行
```

## 🛠️ 构建系统

### CMake构建系统（推荐）

OSZ采用现代化的CMake构建系统，提供以下优势：
- **跨平台支持**：可在Linux、macOS和Windows（WSL）上构建
- **模块化配置**：每个内核模块都是独立的CMake目标
- **灵活的目标选择**：可以单独构建特定模块
- **集成开发环境支持**：支持VSCode、CLion等现代IDE

#### 主要CMake目标
- `target_qemu_all` - 构建完整的QEMU虚拟平台可执行文件
- `target_qemu_run` - 构建并运行QEMU虚拟平台
- `target_qemu_debug` - 构建调试版本（支持GDB）
- `target_qemu_disasm` - 生成反汇编文件
- `k_base_*` - 内核基础模块（任务、内存、调度等）
- `k_*` - 其他内核模块（shell、debug、dfx等）

### Makefile构建系统（传统）

传统的Makefile构建系统提供向后兼容性：
- **简单直接**：熟悉的Makefile工作流程
- **快速构建**：适用于快速原型开发
- **向后兼容**：确保现有工作流程不受影响

## 📁 项目结构

```
osz/
├── arch/              # 体系结构相关代码（RISC-V）
├── build/             # CMake构建输出目录
├── construct/         # 构建系统配置
│   ├── cmake/        # CMake模块配置
│   ├── config/       # 项目配置
│   ├── make/         # Makefile构建配置
│   └── script/       # 构建脚本
├── doc/              # 文档
├── kernel/           # 内核核心
│   ├── base/         # 基础组件（任务、内存、调度等）
│   ├── comm/         # 通用定义和工具
│   ├── debug/        # 调试支持（printf等）
│   ├── dfx/          # 诊断和故障扩展
│   ├── drivers/      # 设备驱动
│   ├── shell/        # Shell实现
│   ├── startup/      # 内核启动代码
│   └── struct/       # 数据结构实现
├── output/           # 构建输出文件
├── platform/         # 平台支持
├── targets/          # 目标板配置
└── tools/            # 工具链和实用工具
```

## 🔧 交叉编译工具链

项目提供了预编译的RISC-V工具链：`tools/riscv-unknown-elf.tar.gz`

工具链源码：[riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
Commit ID: `935b263c8ef7f250819c74aeb7736c87ad87ef2b`

编译配置：
```shell
./configure --prefix=/home/zyzs/code/riscv_tool_chain/install \
            --with-arch=rv32gc \
            --with-abi=ilp32d \
            --enable-default-pie \
            --with-languages=c,c++ \
            --with-cmodel=medany
```

## 📚 文档

### 内核文档
- [启动流程](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/startup.md) - 详细的内核启动过程分析
- [Shell使用指南](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/shell.md) - Shell命令和功能说明
- [已知问题](https://github.com/chenyuxiangg/osz/blob/main/doc/kernel_doc/bugs.md) - 当前版本已知问题和解决方案

### 编译文档
- [编译工具配置](https://github.com/chenyuxiangg/osz/blob/main/doc/compile_doc/compile_tool_config.md) - 构建系统配置指南

### 库函数文档
- [strlen实现](https://github.com/chenyuxiangg/osz/blob/main/doc/libc_doc/strlen实现.md)
- [strncmp实现](https://github.com/chenyuxiangg/osz/blob/main/doc/libc_doc/strncmp实现.md)
- [strncpy实现](https://github.com/chenyuxiangg/osz/blob/main/doc/libc_doc/strncpy实现.md)

## ❓ 常见问题

### 构建问题
**CMake构建失败**
- 确保已安装CMake（版本3.10或更高）
- 清理build目录后重试：`rm -rf build && cd construct && ./build_cmake.sh`

**Makefile构建失败**
- 确保已正确解压工具链到tools/目录
- 检查construct/build.sh执行权限：`chmod +x construct/build.sh`

### 调试信息问题
当使用调试选项（-g）时，链接脚本中不应指定 `.debug_*` 这类段，应保持默认配置。

## 🤝 参与贡献

我们欢迎任何形式的贡献！以下是参与项目的方式：

1. **报告问题**：使用 [GitHub Issues](https://github.com/chenyuxiangg/osz/issues) 报告bug或提出功能请求
2. **提交代码**：通过Pull Request提交改进和修复
3. **完善文档**：帮助改进文档和示例代码
4. **分享经验**：在讨论区分享使用经验和最佳实践

### 贡献流程
1. Fork本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

### 开发规范
- 代码风格遵循项目现有的编码规范
- 提交信息清晰描述变更内容
- 新功能需包含相应的测试用例
- 文档更新需与代码变更同步

## 📄 许可证

本项目采用开源许可证，具体请查看 [LICENSE](LICENSE) 文件。

## 🔗 参考资源

- [Makefile常用函数列表](https://github.com/marmotedu/geekbang-go/blob/master/makefile/Makefile常用函数列表.md)
- [从头写一个RISCV OS](https://github.com/plctlab/riscv-operating-system-mooc) - 优秀的RISC-V操作系统学习资源
- [RT-Thread](https://github.com/RT-Thread/rt-thread) - 优秀的开源RTOS，本项目的设计参考之一

## 📞 联系与支持

- **GitHub仓库**: [https://github.com/chenyuxiangg/osz](https://github.com/chenyuxiangg/osz)
- **问题反馈**: [GitHub Issues](https://github.com/chenyuxiangg/osz/issues)

---

**OSZ** - 简洁高效的RISC-V实时操作系统 | 为嵌入式系统而生
