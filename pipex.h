/* pipex.h - Header file for pipex program */
/* Author: your_login */
/* Description: Contains function prototypes, constants, and structures for pipex */

#ifndef PIPEX_H
# define PIPEX_H

# include <unistd.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <stdbool.h>
# include <stdarg.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 4096
# endif

/* Function prototypes */

/* Mandatory part */
void	execute_pipex(char *file1, char **cmd1, char **cmd2, char *file2);
void	handle_error(char *message);
void	cleanup_resources(int *pipefd, int *infile_fd, int *outfile_fd);
void	execute_command(char **cmd, int pipefd, int input_fd, int output_fd);

/* Bonus part - only available when bonus is compiled */
#ifdef BONUS_FEATURES
void	execute_bonus_pipex(char *file1, char **commands, char *file2, int cmd_count);
void	execute_here_doc(char *limiter, char **commands, char *file, int cmd_count);
void	read_here_doc(char *limiter, int pipefd);
#endif

/* Utility functions */
char	**split_command(char *cmd);
void	cleanup_command(char **cmd);
int		open_file(char *filename, int flags, mode_t mode);
void	close_fd(int fd);
char	*find_command_path(char *cmd);

/* Basic string functions (since we can't use external libft) */
int		ft_strcmp(const char *s1, const char *s2);
size_t	ft_strlen(const char *s);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
char	*ft_strchr(const char *s, int c);
char	*ft_strdup(const char *s);
void	*ft_calloc(size_t count, size_t size);
void	*ft_memset(void *s, int c, size_t n);
int		ft_printf(const char *format, ...);

#endif