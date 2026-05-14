# Pipex · 三语说明 / Trilingual README / README Trilingüe

> 42 School project: reproduce Unix piping behavior in C using `pipe`, `fork`, `dup2`, and `execve`.

---

## 中文（普通话）

### 项目简介
`pipex` 复现以下 shell 行为：

```bash
< file1 cmd1 | cmd2 > file2
```

对应执行方式：

```bash
./pipex file1 "cmd1" "cmd2" file2
```

Bonus 支持：
- 多命令管道：`./pipex file1 cmd1 cmd2 ... cmdn file2`
- heredoc：`./pipex here_doc LIMITER cmd1 cmd2 file`

### 项目结构
- `src/`：必做部分源码
- `bonus/`：Bonus 源码
- `includes/`：头文件
- `libft/`：静态库依赖
- `Makefile`：构建规则

### 编译
```bash
make
make bonus
make re
```

### 使用示例
```bash
./pipex infile "grep hello" "wc -l" outfile
```

### 常见要求
- 正确处理进程与文件描述符
- 避免内存泄漏
- 处理系统调用错误（`open`, `pipe`, `fork`, `dup2`, `execve`）

---

## English

### Overview
`pipex` reproduces this shell behavior:

```bash
< file1 cmd1 | cmd2 > file2
```

Equivalent program usage:

```bash
./pipex file1 "cmd1" "cmd2" file2
```

Bonus features:
- Multiple commands: `./pipex file1 cmd1 cmd2 ... cmdn file2`
- heredoc mode: `./pipex here_doc LIMITER cmd1 cmd2 file`

### Project Layout
- `src/`: mandatory implementation
- `bonus/`: bonus implementation
- `includes/`: headers
- `libft/`: static library dependency
- `Makefile`: build targets

### Build
```bash
make
make bonus
make re
```

### Example
```bash
./pipex infile "grep hello" "wc -l" outfile
```

### Key Expectations
- Correct process and file descriptor handling
- No memory leaks
- Robust syscall error handling (`open`, `pipe`, `fork`, `dup2`, `execve`)

---

## Español

### Resumen
`pipex` reproduce este comportamiento de shell:

```bash
< file1 cmd1 | cmd2 > file2
```

Uso equivalente del programa:

```bash
./pipex file1 "cmd1" "cmd2" file2
```

Funcionalidades bonus:
- Múltiples comandos: `./pipex file1 cmd1 cmd2 ... cmdn file2`
- Modo heredoc: `./pipex here_doc LIMITER cmd1 cmd2 file`

### Estructura del proyecto
- `src/`: implementación obligatoria
- `bonus/`: implementación bonus
- `includes/`: cabeceras
- `libft/`: dependencia de biblioteca estática
- `Makefile`: reglas de compilación

### Compilación
```bash
make
make bonus
make re
```

### Ejemplo
```bash
./pipex infile "grep hello" "wc -l" outfile
```

### Requisitos clave
- Manejo correcto de procesos y descriptores de archivo
- Sin fugas de memoria
- Gestión sólida de errores de syscalls (`open`, `pipe`, `fork`, `dup2`, `execve`)

---

## License
This project is under the MIT License. See `LICENSE`.
