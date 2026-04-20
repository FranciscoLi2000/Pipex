#include "pipex_bonus.h"

void	child_process(int fd_in, int fd_out, char *cmd, char **envp)
{
	dup2(fd_in, STDIN_FILENO);
	dup2(fd_out, STDOUT_FILENO);
	close(fd_in);
	close(fd_out);
	exec_cmd(cmd, envp);
}

static void	pipe_child(int in_fd, int *fd, char *cmd, char **envp)
{
	pid_t	pid;

	if (pipe(fd) == -1)
		err_exit(ERR_PIPE);
	pid = fork();
	if (pid == -1)
		err_exit(ERR_FORK);
	if (pid == 0)
	{
		close(fd[0]);
		child_process(in_fd, fd[1], cmd, envp);
	}
}

static int	setup_pipeline(int ac, char **av, char **envp)
{
	int		fd[2];
	int		in_fd;
	int		i;

	in_fd = open(av[1], O_RDONLY);
	if (in_fd == -1)
		err_exit(ERR_OPEN);
	i = 2;
	while (i < ac - 2)
	{
		pipe_child(in_fd, fd, av[i], envp);
		close(fd[1]);
		close(in_fd);
		in_fd = fd[0];
		i++;
	}
	return (in_fd);
}

static void	last_child(int in_fd, char *outfile, char *cmd, char **envp)
{
	int		out_fd;
	pid_t	pid;

	out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (out_fd == -1)
		err_exit(ERR_OPEN);
	pid = fork();
	if (pid == -1)
		err_exit(ERR_FORK);
	if (pid == 0)
		child_process(in_fd, out_fd, cmd, envp);
	close(in_fd);
	close(out_fd);
}

void	pipex_multiple(int ac, char **av, char **envp)
{
	int	in_fd;

	in_fd = setup_pipeline(ac, av, envp);
	last_child(in_fd, av[ac - 1], av[ac - 2], envp);
	while (wait(NULL) > 0)
		;
}

static void	here_doc_mode(char **av, char **envp)
{
	int		in_fd;
	int		out_fd;
	int		fd[2];
	pid_t	pid1;
	pid_t	pid2;

	here_doc(av[2]);
	in_fd = open(HEREDOC_TMP, O_RDONLY);
	if (in_fd == -1)
		err_exit(ERR_OPEN);
	out_fd = open(av[5], O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (out_fd == -1)
		err_exit(ERR_OPEN);
	if (pipe(fd) == -1)
		err_exit(ERR_PIPE);
	pid1 = fork();
	if (pid1 == -1)
		err_exit(ERR_FORK);
	if (pid1 == 0)
	{
		close(fd[0]);
		child_process(in_fd, fd[1], av[3], envp);
	}
	pid2 = fork();
	if (pid2 == -1)
		err_exit(ERR_FORK);
	if (pid2 == 0)
	{
		close(fd[1]);
		child_process(fd[0], out_fd, av[4], envp);
	}
	close(fd[0]);
	close(fd[1]);
	close(in_fd);
	close(out_fd);
	wait(NULL);
	wait(NULL);
	unlink(HEREDOC_TMP);
}

int	main(int ac, char **av, char **envp)
{
	if (ac < 5)
	{
		ft_putstr_fd(ERR_ARGS, 2);
		return (EXIT_FAILURE);
	}
	if (ft_strcmp(av[1], "here_doc") == 0)
	{
		if (ac != 6)
		{
			ft_putstr_fd(ERR_ARGS, 2);
			return (EXIT_FAILURE);
		}
		here_doc_mode(av, envp);
	}
	else
		pipex_multiple(ac, av, envp);
	return (EXIT_SUCCESS);
}
