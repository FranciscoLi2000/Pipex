#ifndef PIPEX_BONUS_H
# define PIPEX_BONUS_H

# include <unistd.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
# include "libft.h"

# define ERR_ARGS	"Usage:\n  ./pipex file1 cmd1 ... cmdn file2\n" \
					"  ./pipex here_doc LIMITER cmd1 cmd2 file\n"
# define ERR_PIPE	"pipe"
# define ERR_FORK	"fork"
# define ERR_OPEN	"open"
# define ERR_CMD	"command not found"
# define ERR_EXEC	"execve"
# define HEREDOC_TMP	".heredoc_tmp"

/* ft_error_bonus.c */
void	err_exit(char *msg);
void	free_tab(char **tab);

/* ft_path_bonus.c */
char	*get_path(char *cmd, char **envp);

/* ft_exec_bonus.c */
void	exec_cmd(char *arg, char **envp);

/* ft_here_doc_bonus.c */
void	here_doc(char *limiter);

/* pipex_bonus.c */
void	child_process(int fd_in, int fd_out, char *cmd, char **envp);
void	pipex_multiple(int ac, char **av, char **envp);

#endif
