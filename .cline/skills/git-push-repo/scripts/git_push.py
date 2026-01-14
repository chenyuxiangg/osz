#!/usr/bin/env python3
"""
Git Push Script

A script to automate git commit and push operations with proper error handling.
通用Git提交和推送脚本，适用于任何Git远程仓库（GitHub、GitLab、Gitee等）。
"""

import subprocess
import sys
import os
from typing import Optional, Tuple

def run_command(cmd: list, cwd: Optional[str] = None) -> Tuple[bool, str]:
    """
    Run a shell command and return success status and output.
    """
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            capture_output=True,
            text=True,
            check=False
        )
        return result.returncode == 0, result.stdout.strip()
    except Exception as e:
        return False, str(e)

def check_git_repo(path: str = ".") -> bool:
    """
    Check if the given path is a git repository.
    """
    success, _ = run_command(["git", "rev-parse", "--git-dir"], cwd=path)
    return success

def get_current_branch(path: str = ".") -> Optional[str]:
    """
    Get the current branch name.
    """
    success, output = run_command(["git", "branch", "--show-current"], cwd=path)
    if success:
        return output
    return None

def get_status(path: str = ".") -> str:
    """
    Get git status.
    """
    success, output = run_command(["git", "status", "--short"], cwd=path)
    if success:
        return output
    return ""

def has_changes(path: str = ".") -> bool:
    """
    Check if there are any uncommitted changes.
    """
    status = get_status(path)
    return len(status) > 0

def git_add_all(path: str = ".") -> bool:
    """
    Stage all changes.
    """
    success, _ = run_command(["git", "add", "."], cwd=path)
    return success

def git_commit(message: str, path: str = ".") -> bool:
    """
    Commit changes with the given message.
    """
    success, _ = run_command(["git", "commit", "-m", message], cwd=path)
    return success

def git_push(branch: Optional[str] = None, remote: str = "origin", path: str = ".") -> bool:
    """
    Push to remote repository.
    """
    if branch is None:
        branch = get_current_branch(path)
        if branch is None:
            print("错误：无法确定当前分支")
            return False
    
    success, output = run_command(["git", "push", remote, branch], cwd=path)
    return success

def main():
    """
    Main function: parse arguments and execute git operations.
    """
    import argparse
    
    parser = argparse.ArgumentParser(description="Git commit and push automation. 通用Git提交和推送自动化脚本。")
    parser.add_argument("--message", "-m", required=True, help="提交信息（必需）")
    parser.add_argument("--branch", "-b", help="要推送到的分支（默认：当前分支）")
    parser.add_argument("--remote", "-r", default="origin", help="远程仓库名称（默认：origin）")
    parser.add_argument("--path", "-p", default=".", help="Git仓库路径（默认：当前目录）")
    parser.add_argument("--no-push", action="store_true", help="仅提交，不推送")
    parser.add_argument("--dry-run", action="store_true", help="显示将要执行的操作而不实际执行")
    
    args = parser.parse_args()
    
    # Check if path exists
    if not os.path.exists(args.path):
        print(f"错误：路径 '{args.path}' 不存在")
        sys.exit(1)
    
    # Check if it's a git repo
    if not check_git_repo(args.path):
        print(f"错误：'{args.path}' 不是一个Git仓库")
        sys.exit(1)
    
    # Check for changes
    if not has_changes(args.path):
        print("没有要提交的更改")
        sys.exit(0)
    
    print(f"仓库路径：{os.path.abspath(args.path)}")
    current_branch = get_current_branch(args.path)
    print(f"当前分支：{current_branch}")
    
    status = get_status(args.path)
    if status:
        print("待提交的更改：")
        print(status)
    
    if args.dry_run:
        print(f"干跑模式：将提交信息为 '{args.message}' 的更改")
        if not args.no_push:
            print(f"将推送到 {args.remote}/{args.branch or current_branch}")
        sys.exit(0)
    
    # Stage changes
    print("暂存更改...")
    if not git_add_all(args.path):
        print("错误：暂存更改失败")
        sys.exit(1)
    
    # Commit
    print(f"提交信息：{args.message}")
    if not git_commit(args.message, args.path):
        print("错误：提交失败")
        sys.exit(1)
    
    # Push
    if not args.no_push:
        print(f"推送到 {args.remote}/{args.branch or current_branch}...")
        if not git_push(args.branch, args.remote, args.path):
            print("错误：推送失败")
            sys.exit(1)
        print("推送成功")
    else:
        print("提交成功（未推送）")
    
    print("完成！")

if __name__ == "__main__":
    main()
