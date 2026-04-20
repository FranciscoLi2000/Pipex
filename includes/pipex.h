#ifndef PIPEX_H
# define PIPEX_H

# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include "libft.h"

# define ERR_ARGS	"Usage: ./pipex file1 cmd1 cmd2 file2\n"
# define ERR_PIPE	"pipe"
# define ERR_FORK	"fork"
# define ERR_OPEN	"open"
# define ERR_CMD	"command not found"
# define ERR_EXEC	"execve"

/* ft_error.c */
void	err_exit(char *msg);
void	free_tab(char **tab);

/* ft_path.c */
char	*get_path(char *cmd, char **envp);

/* ft_exec.c */
void	exec_cmd(char *arg, char **envp);

/* pipex.c */
void	child1(int fd[2], char **argv, char **envp);
void	child2(int fd[2], char **argv, char **envp);

#endif
