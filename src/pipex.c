#include "pipex.h"

void	child1(int fd[2], char **argv, char **envp)
{
	int	infile;

	infile = open(argv[1], O_RDONLY);
	if (infile == -1)
		err_exit(ERR_OPEN);
	dup2(infile, STDIN_FILENO);
	dup2(fd[1], STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(infile);
	exec_cmd(argv[2], envp);
}

void	child2(int fd[2], char **argv, char **envp)
{
	int	outfile;

	outfile = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outfile == -1)
		err_exit(ERR_OPEN);
	dup2(fd[0], STDIN_FILENO);
	dup2(outfile, STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(outfile);
	exec_cmd(argv[3], envp);
}

int	main(int argc, char **argv, char **envp)
{
	int		fd[2];
	pid_t	pid1;
	pid_t	pid2;

	if (argc != 5)
	{
		ft_putstr_fd(ERR_ARGS, 2);
		return (EXIT_FAILURE);
	}
	if (pipe(fd) == -1)
		err_exit(ERR_PIPE);
	pid1 = fork();
	if (pid1 == -1)
		err_exit(ERR_FORK);
	if (pid1 == 0)
		child1(fd, argv, envp);
	pid2 = fork();
	if (pid2 == -1)
		err_exit(ERR_FORK);
	if (pid2 == 0)
		child2(fd, argv, envp);
	close(fd[0]);
	close(fd[1]);
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
	return (EXIT_SUCCESS);
}
