/* execute.c - Core execution logic for basic pipex */
/* Author: your_login */
/* Description: Implements the basic two-command pipeline */

#include "pipex.h"

/**
 * execute_pipex - Execute basic pipex with two commands
 * @file1: Input file path
 * @cmd1: First command and arguments
 * @cmd2: Second command and arguments
 * @file2: Output file path
 *
 * This function implements the core logic:
 * 1. Create pipe for inter-process communication
 * 2. Fork first child to execute cmd1
 * 3. Fork second child to execute cmd2
 * 4. First child's output goes to pipe, second reads from pipe
 * 5. Wait for both children to complete
 */
void	execute_pipex(char *file1, char **cmd1, char **cmd2, char *file2)
{
	int		pipefd[2];      // File descriptors for the pipe
	int		infile_fd;      // File descriptor for input file
	int		outfile_fd;     // File descriptor for output file
	pid_t	pid1;          // Process ID for first child
	pid_t	pid2;          // Process ID for second child
	
	/* Step 1: Create the pipe */
	if (pipe(pipefd) == -1)
	{
		handle_error("pipe creation failed");
		return;
	}
	
	/* Step 2: Open input file for reading */
	infile_fd = open_file(file1, O_RDONLY, 0);
	if (infile_fd == -1)
	{
		cleanup_resources(pipefd, &infile_fd, &outfile_fd);
		return;
	}
	
	/* Step 3: Create or truncate output file for writing */
	outfile_fd = open_file(file2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outfile_fd == -1)
	{
		cleanup_resources(pipefd, &infile_fd, &outfile_fd);
		return;
	}
	
	/* Step 4: Create first child process to execute cmd1 */
	pid1 = fork();
	if (pid1 == -1)
	{
		handle_error("fork failed");
		cleanup_resources(pipefd, &infile_fd, &outfile_fd);
		return;
	}
	
	if (pid1 == 0)  // First child process
	{
		/* Child 1: Execute cmd1 */
		
		/* Redirect stdout to write end of pipe */
		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
		{
			handle_error("dup2 failed for child1 stdout");
			exit(1);
		}
		
		/* Redirect stdin to read input file */
		if (dup2(infile_fd, STDIN_FILENO) == -1)
		{
			handle_error("dup2 failed for child1 stdin");
			exit(1);
		}
		
		/* Close unused file descriptors */
		close_fd(pipefd[0]);
		close_fd(pipefd[1]);
		close_fd(infile_fd);
		close_fd(outfile_fd);
		
		/* Execute first command */
		char	*cmd1_path = find_command_path(cmd1[0]);
		if (!cmd1_path)
		{
			handle_error("command not found");
			exit(1);
		}
		cmd1[0] = cmd1_path;
		execve(cmd1[0], cmd1, NULL);
		
		/* If execve returns, it means the command failed */
		handle_error("execve failed for first command");
		exit(1);
	}
	
	/* Step 5: Create second child process to execute cmd2 */
	pid2 = fork();
	if (pid2 == -1)
	{
		handle_error("fork failed");
		cleanup_resources(pipefd, &infile_fd, &outfile_fd);
		return;
	}
	
	if (pid2 == 0)  // Second child process
	{
		/* Child 2: Execute cmd2 */
		
		/* Redirect stdin to read from pipe */
		if (dup2(pipefd[0], STDIN_FILENO) == -1)
		{
			handle_error("dup2 failed for child2 stdin");
			exit(1);
		}
		
		/* Redirect stdout to write to output file */
		if (dup2(outfile_fd, STDOUT_FILENO) == -1)
		{
			handle_error("dup2 failed for child2 stdout");
			exit(1);
		}
		
		/* Close unused file descriptors */
		close_fd(pipefd[0]);
		close_fd(pipefd[1]);
		close_fd(infile_fd);
		close_fd(outfile_fd);
		
		/* Execute second command */
		char	*cmd2_path = find_command_path(cmd2[0]);
		if (!cmd2_path)
		{
			handle_error("command not found");
			exit(1);
		}
		cmd2[0] = cmd2_path;
		execve(cmd2[0], cmd2, NULL);
		
		/* If execve returns, it means the command failed */
		handle_error("execve failed for second command");
		exit(1);
	}
	
	/* Step 6: Parent process - close all file descriptors and wait for children */
	close_fd(pipefd[0]);
	close_fd(pipefd[1]);
	close_fd(infile_fd);
	close_fd(outfile_fd);
	
	/* Wait for both children to complete */
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
}

/**
 * execute_command - Execute a single command (used by bonus features)
 * @cmd: Command and arguments
 * @pipefd: Pipe file descriptors
 * @input_fd: Input file descriptor
 * @output_fd: Output file descriptor
 */
void	execute_command(char **cmd, int pipefd, int input_fd, int output_fd)
{
	pid_t	pid;
	
	pid = fork();
	if (pid == -1)
	{
		handle_error("fork failed");
		return;
	}
	
	if (pid == 0)  // Child process
	{
		/* Redirect stdin if input_fd is provided */
		if (input_fd != -1 && dup2(input_fd, STDIN_FILENO) == -1)
		{
			handle_error("dup2 failed for stdin");
			exit(1);
		}
		
		/* Redirect stdout if output_fd is provided */
		if (output_fd != -1 && dup2(output_fd, STDOUT_FILENO) == -1)
		{
			handle_error("dup2 failed for stdout");
			exit(1);
		}
		
		/* Close pipe file descriptors */
		close_fd(pipefd);
		close_fd(input_fd);
		close_fd(output_fd);
		
		/* Execute command */
		char	*cmd_path = find_command_path(cmd[0]);
		if (!cmd_path)
		{
			handle_error("command not found");
			exit(1);
		}
		cmd[0] = cmd_path;
		execve(cmd[0], cmd, NULL);
		
		/* If execve returns, command failed */
		handle_error("execve failed");
		exit(1);
	}
}