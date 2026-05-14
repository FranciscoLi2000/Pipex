# Pipex

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)

Pipex is a C program that reproduces Unix shell pipelines by connecting commands with pipes and managing input/output files, with optional heredoc support.

## Table of Contents

- [Project Introduction](#project-introduction)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing Guide](#contributing-guide)
- [License](#license)

## Project Introduction

Pipex is a 42 School project focused on process management and inter-process communication. It mirrors this shell behavior:

```bash
< infile cmd1 | cmd2 > outfile
```

**Tech Stack**
- C (POSIX APIs: `pipe`, `fork`, `dup2`, `execve`)
- Make (build automation)
- libft (static utility library)

**Key Features**
- Executes a two-command pipeline with input/output redirection
- Bonus: supports multiple commands in a pipeline
- Bonus: supports `here_doc` for heredoc-style input
- Robust error handling for system calls and file operations

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/FranciscoLi2000/Pipex.git
   ```
2. Enter the project directory:
   ```bash
   cd Pipex
   ```
3. Build the mandatory version:
   ```bash
   make
   ```
4. (Optional) Build the bonus version:
   ```bash
   make bonus
   ```
5. (Optional) Clean build artifacts:
   ```bash
   make clean
   make fclean
   make re
   ```

## Usage

### Mandatory usage

```bash
./pipex infile "cmd1" "cmd2" outfile
```

Example:

```bash
./pipex infile "grep hello" "wc -l" outfile
```

### Bonus usage

Multiple commands:

```bash
./pipex infile "cmd1" "cmd2" "cmd3" outfile
```

Heredoc mode:

```bash
./pipex here_doc LIMITER "cmd1" "cmd2" outfile
```

## Contributing Guide

Contributions are welcome. To contribute:

1. Fork the repository and create a feature branch.
2. Make your changes with clear, focused commits.
3. Run `make` (and `make bonus` if relevant) to verify builds.
4. Open a pull request describing the changes and rationale.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
