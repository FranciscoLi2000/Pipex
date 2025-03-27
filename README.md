# Pipex

这个 **Pipex 项目** 旨在通过实现一个简化的 Unix 管道和重定向工具，帮助你掌握以下 **核心技能** 和 **系统编程知识**：

---

### **核心学习目标**
1. **Unix 进程与管道机制**
- 使用 `fork()` 创建子进程执行命令，结合 `execve()` 加载程序，理解进程的创建与替换。
- 通过 `pipe()` 创建管道，连接多个进程的输入输出（如 `cmd1 | cmd2`），掌握进程间通信（IPC）的基础。

2. **文件描述符与重定向**
- 使用 `dup2()` 重定向标准输入/输出（如 `< infile` 和 `> outfile`），理解文件描述符的底层操作。
- 处理文件打开（`open()`）、关闭（`close()`）和错误（如文件不存在或权限不足）。

3. **命令行参数解析**
- 将字符串命令（如 `"ls -l"`）解析为可执行路径和参数列表（`argv`），适配 `execve()` 的调用格式。
- 使用 `access()` 检查命令的可执行性，处理 `PATH` 环境变量搜索（如通过 `env` 或自定义逻辑）。

4. **错误处理与健壮性**
- 捕获系统调用错误（如 `perror()` 和 `strerror()`），确保程序在无效输入、命令执行失败或管道创建失败时优雅退出。
- 避免文件描述符泄漏（如未关闭的管道或文件句柄），释放所有动态分配的内存（`free()`）。

5. **多进程管理与同步**
- 控制父子进程的执行顺序（如父进程等待子进程结束的 `waitpid()`），避免僵尸进程。
- 在 Bonus 中扩展支持多管道（如 `cmd1 | cmd2 | cmd3`），动态管理多个管道和进程链。

6. **Here Document 处理（Bonus）**
- 实现 `<< LIMITADOR` 功能，从标准输入读取数据直到遇到指定分隔符，生成临时文件或内存缓冲区作为输入源。
- 支持 `>>` 追加输出模式，使用 `O_APPEND` 标志打开文件。

---

### **关键技术点解析**
#### 1. 基本管道与重定向实现
```c
	// 创建管道
	int	pipefd[2];
	pipe(pipefd);

	// 执行 cmd1：将输入重定向到 infile，输出写入管道
	if (fork() == 0)
	{
		close(pipefd[0]);	// 关闭读端
		dup2(input_fd, STDIN_FILENO);    // 输入重定向
		dup2(pipefd[1], STDOUT_FILENO);  // 输出重定向到管道
		execve(cmd1_path, cmd1_args, env);
	}

	// 执行 cmd2：从管道读取输入，输出重定向到 outfile
	if (fork() == 0)
	{
		close(pipefd[1]);       // 关闭写端
		dup2(pipefd[0], STDIN_FILENO);   // 输入从管道读取
		dup2(output_fd, STDOUT_FILENO);  // 输出重定向到文件
		execve(cmd2_path, cmd2_args, env);
	}

	// 父进程关闭管道并等待子进程
	close(pipefd[0]);
	close(pipefd[1]);
	waitpid(cmd1_pid, NULL, 0);
	waitpid(cmd2_pid, NULL, 0);
```
**关键点**：通过两次 `fork()` 分别执行命令，通过 `dup2()` 控制输入输出流。

#### 2. 多管道扩展（Bonus）
```c
	int **pipes = malloc((n_cmds - 1) * sizeof(int*));
	for (int i = 0; i < n_cmds - 1; i++)
	{
		pipes[i] = malloc(2 * sizeof(int));
		pipe(pipes[i]);
	}
	for (int i = 0; i < n_cmds; i++)
	{
		if (fork() == 0)
		{
			// 第一个命令：输入重定向到文件
			if (i == 0)
				dup2(input_fd, STDIN_FILENO);
			// 中间命令：输入来自前一个管道，输出到下一个管道
			if (i > 0)
				dup2(pipes[i-1][0], STDIN_FILENO);
			if (i < n_cmds - 1)
				dup2(pipes[i][1], STDOUT_FILENO);
			// 最后一个命令：输出重定向到文件
			if (i == n_cmds - 1)
				dup2(output_fd, STDOUT_FILENO);
			execve(cmd_paths[i], cmd_args[i], env);
		}
	}

	// 关闭所有管道并等待子进程
	for (int i = 0; i < n_cmds - 1; i++)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
		free(pipes[i]);
	}
	free(pipes);
```
**关键点**：动态创建和管理多个管道，确保每个进程正确连接前后管道。

#### 3. Here Document 处理（Bonus）
```c
	if (strcmp(argv[1], "here_doc") == 0)
	{
		char *limiter = argv[2];
		char *line = NULL;
		size_t len = 0;
		int here_pipe[2];
		pipe(here_pipe);
		while (1)
		{
			write(1, "heredoc> ", 9);
			ssize_t read_len = getline(&line, &len, stdin);
			if (read_len <= 0 || strncmp(line, limiter, strlen(limiter)) == 0)
				break ;
			write(here_pipe[1], line, read_len);
		}
		close(here_pipe[1]);
		input_fd = here_pipe[0]; // 将临时管道作为输入源
		free(line);
	}
```
**关键点**：读取标准输入直到匹配分隔符，通过管道或临时文件传递数据。

---

### **工程实践与思维提升**
1. **模块化设计**
- 分离参数解析、命令执行、错误处理到独立函数，提升代码可维护性。
- 使用头文件（如 `pipex.h`）定义共享数据结构和常量。

2. **防御式编程**
- 检查所有系统调用返回值（如 `pipe()`、`fork()`、`execve()`），处理 `ENOENT`（命令不存在）和 `EACCES`（权限不足）等错误。
- 使用 `valgrind` 检测内存泄漏，确保所有 `malloc` 均有对应的 `free`。

3. **测试策略**
- 覆盖边界条件：空文件、无效命令、多个命令链、大型数据流。
- 对比程序输出与原生 Shell 命令（如 `diff`），确保行为一致。

---

### **项目隐喻与深层价值**
- **“管道是Unix哲学的缩影”**：通过简单工具的组合解决复杂问题，体现模块化和可扩展性。
- **“进程管理是系统编程的核心”**：深入理解进程生命周期、资源管理和并发控制，为开发复杂系统（如服务器、分布式应用）奠定基础。
- **“从命令行到系统调用”**：将高级Shell功能转化为底层C实现，强化对操作系统接口的理解。

---

### **通关要点**
1. **分步实现**
- 先实现基础功能（单管道+重定向），再扩展多管道和Here Document。
- 使用 `ft_printf` 或自定义日志输出调试信息。

2. **错误处理全覆盖**
- 处理无效参数（如参数不足、文件不可读）、命令执行失败、管道创建失败等场景。
- 确保错误信息输出到 `stderr`，符合Unix工具标准。

3. **性能优化**
- 避免不必要的进程创建（如提前检查命令是否存在）。
- 优化文件描述符管理，减少冗余操作。

---

通过此项目，你将从“脚本使用者”进阶为“系统开发者”，掌握Unix环境下的核心编程范式，为运维工具开发、后台服务构建提供坚实基础。
