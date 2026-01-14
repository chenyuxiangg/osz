---
name: git-push-repo
description: 提供自动化推送代码到远程仓库的完整工作流，包括状态检查、暂存、提交、推送、分支管理和冲突处理。当用户需要将本地代码更改推送到远程仓库时使用此技能，适用于以下场景：(1) 常规提交和推送，(2) 创建新分支并推送，(3) 处理提交历史冲突，(4) 自动化多步 Git 操作，(5) 生成规范的提交消息。适用于任何 Git 远程仓库（GitHub、GitLab、Gitee 等）。
---

# Git 推送代码仓技能

## 概述

本技能提供了一套完整的 Git 操作自动化工具，专注于将代码推送到远程仓库。它包含了 Python 脚本、最佳实践指南和常见工作流，帮助用户高效、规范地将代码更改推送到任意远程 Git 仓库。

## 快速开始

### 基本提交和推送

对于大多数情况，使用提供的 Python 脚本进行一次性提交和推送：

```bash
# 提交所有更改并推送到当前分支
python scripts/git_push.py -m "你的提交信息"

# 提交但不推送（仅本地提交）
python scripts/git_push.py -m "你的提交信息" --no-push

# 推送到指定远程和分支
python scripts/git_push.py -m "你的提交信息" --remote origin --branch main

# 查看将要执行的操作（干跑模式）
python scripts/git_push.py -m "你的提交信息" --dry-run
```

### 手动 Git 命令参考

如果更喜欢直接使用 Git 命令，以下是标准工作流：

```bash
# 1. 检查状态
git status

# 2. 添加更改
git add .              # 添加所有更改
git add <文件路径>     # 添加特定文件

# 3. 提交
git commit -m "清晰的提交信息"

# 4. 推送
git push <远程名称> <分支名>
```

## 核心工作流

### 1. 常规提交流程

这是最常见的场景：将本地更改提交到当前分支并推送到远程。

**步骤：**
1. 检查当前分支和状态
2. 暂存所有更改（或特定文件）
3. 编写有意义的提交消息
4. 提交更改
5. 推送到远程仓库

**使用脚本：**
```bash
python scripts/git_push.py -m "feat: 添加新功能"
```

### 2. 创建新分支并推送

当需要在新分支上开发功能时：

```bash
# 创建并切换到新分支
git checkout -b feature/new-feature

# 进行更改并提交
python scripts/git_push.py -m "feat: 开始新功能开发" --branch feature/new-feature

# 首次推送需要设置上游分支
git push --set-upstream origin feature/new-feature
```

### 3. 处理提交冲突

如果远程有更新导致推送失败：

```bash
# 1. 拉取最新更改
git pull <远程名称> <分支名>

# 2. 解决冲突（手动编辑文件）
# 3. 重新提交
git add .
git commit -m "merge: 解决合并冲突"
git push <远程名称> <分支名>
```

### 4. 修改最近提交

如果需要修改最近一次提交：

```bash
# 修改提交信息
git commit --amend -m "新的提交信息"

# 添加漏掉的文件并修改提交
git add <漏掉的文件>
git commit --amend --no-edit

# 强制推送（谨慎使用）
git push --force-with-lease <远程名称> <分支名>
```

## 提交消息规范

良好的提交消息有助于团队协作和版本管理。推荐使用 Conventional Commits 格式：

```
<类型>[可选范围]: <描述>

[可选正文]

[可选脚注]
```

**常见类型：**
- `feat`: 新功能
- `fix`: 错误修复
- `docs`: 文档更新
- `style`: 代码格式调整（不影响功能）
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建过程或辅助工具变动

**示例：**
```
fix: 修复内存泄漏问题

- 修复了循环引用导致的内存泄漏
- 添加了内存使用监控

Closes #123
```

## 高级用法

### 多远程仓库操作

如果项目有多个远程仓库：

```bash
# 查看所有远程仓库
git remote -v

# 推送到特定远程
git push upstream main
git push origin main

# 一次性推送到多个远程
git remote | xargs -I {} git push {} main
```

### 批量提交

对于多个相关但独立的更改，可以考虑分开提交：

```bash
# 交互式暂存，选择部分更改
git add -p

# 分别提交不同的更改
git commit -m "feat: 添加用户模型"
git add -p  # 继续选择其他更改
git commit -m "docs: 更新 API 文档"
```

### 自动化脚本集成

将 Git 操作集成到 CI/CD 流水线或自动化脚本中：

```python
#!/usr/bin/env python3
import subprocess
import sys

def auto_commit_if_changes(commit_msg, remote="origin", branch="main"):
    """检查是否有更改并自动提交"""
    result = subprocess.run(['git', 'status', '--porcelain'], 
                          capture_output=True, text=True)
    if result.stdout.strip():
        subprocess.run(['git', 'add', '.'])
        subprocess.run(['git', 'commit', '-m', commit_msg])
        subprocess.run(['git', 'push', remote, branch])
        return True
    return False
```

## 故障排除

### 常见错误及解决方案

1. **错误：`fatal: not a git repository`**
   - 解决方案：确保在当前目录或父目录中存在 `.git` 文件夹
   - 检查：`git rev-parse --git-dir`

2. **错误：`Updates were rejected because the remote contains work`**
   - 解决方案：先拉取远程更改 `git pull <远程名称> <分支名>`
   - 或使用：`git pull --rebase <远程名称> <分支名>`

3. **错误：`Authentication failed`**
   - 解决方案：检查 Git 凭证，使用 `git config --list` 查看配置
   - 重新配置：`git config --global credential.helper cache`

4. **错误：`branch name too long`**
   - 解决方案：分支名应少于 40 个字符，使用有意义的简短名称

### 脚本调试

如果脚本运行异常，启用详细输出：

```bash
# 查看脚本帮助
python scripts/git_push.py --help

# 启用详细日志（修改脚本添加 verbose 参数）
import logging
logging.basicConfig(level=logging.DEBUG)
```

## 资源

### scripts/git_push.py

主要自动化脚本，提供以下功能：
- 检查 Git 仓库状态
- 暂存所有更改
- 提交并推送
- 干跑模式预览
- 错误处理和状态报告

**参数说明：**
- `-m, --message`: 提交消息（必需）
- `-b, --branch`: 目标分支（默认：当前分支）
- `-r, --remote`: 远程名称（默认：origin）
- `-p, --path`: Git 仓库路径（默认：当前目录）
- `--no-push`: 仅提交，不推送
- `--dry-run`: 预览将要执行的操作

**示例：**
```bash
# 完整工作流
python scripts/git_push.py -m "修复登录页面样式" -b main -r origin

# 仅本地提交
python scripts/git_push.py -m "WIP: 开发中功能" --no-push

# 指定仓库路径
python scripts/git_push.py -m "更新配置" -p /path/to/repo
```

## 最佳实践

1. **频繁提交**：小而专注的提交更容易理解和回滚
2. **描述性消息**：提交消息应清晰说明更改内容和原因
3. **分支策略**：使用功能分支开发，主分支保持稳定
4. **先拉后推**：推送前先拉取远程更新，避免冲突
5. **代码审查**：利用 Pull Request 功能进行代码审查

## 延伸学习

- [Git 官方文档](https://git-scm.com/doc)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Git 分支策略比较](https://www.atlassian.com/git/tutorials/comparing-workflows)

---

**提示**：本技能提供的脚本和指南适用于大多数常见场景。对于复杂的 Git 操作（如交互式变基、二分查找等），建议直接使用 Git 命令或参考高级 Git 教程。
