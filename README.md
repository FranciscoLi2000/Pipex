# Pipex 项目 - Unix 管道编程

## 项目概述
pipex 是一个模拟 Unix 管道操作的程序，通过系统调用实现进程间通信和管道功能。

## 功能实现

### Mandatory 部分
- 实现基本的双命令管道：`file1 cmd1 cmd2 file2`
- 等价于 shell 命令：`file1 | cmd1 cmd2 > file2`

### Bonus 部分  
1. **多管道支持**：`file1 cmd1 cmd2 cmd3 ... cmdn file2`
2. **here_doc 支持**：`pipex here_doc LIMITER cmd cmd1 ... cmdn file`

## 核心系统调用
- `fork()`: 创建子进程
- `pipe()`: 创建管道
- `dup2()`: 文件描述符重定向
- `execve()`: 执行外部命令
- `wait()`: 等待子进程

## 编译和使用
```bash
make          # 编译 mandatory 部分
make bonus   # 编译 bonus 部分  
make clean   # 清理目标文件
```

## 使用示例
```bash
# 基本用法
./pipex infile "ls -l" "wc -l" outfile
# 等价于: infile | ls -l | wc -l > outfile

# 多管道
./pipex infile "cat" "grep hello" "wc -l" outfile
# 等价于: infile | cat | grep hello | wc -l > outfile

# here_doc
./pipex here_doc LIMITER "cat" "grep pattern" outfile
# 等价于: cat << LIMITER | grep pattern > outfile
```
