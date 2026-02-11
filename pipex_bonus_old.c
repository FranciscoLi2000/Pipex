/* pipex_bonus.c - Bonus part for multiple pipes */
/* Author: your_login */
/* Description: Implements multiple pipe functionality */

#include "pipex.h"

/**
 * execute_bonus_pipex - Execute pipex with multiple commands
 * @file1: Input file path
 * @commands: Array of command arrays
 * @file2: Output file path
 * @cmd_count: Number of commands
 *
 * This function implements the logic for multiple pipes:
 * file1 | cmd1 | cmd2 | cmd3 | ... | cmdn > file2
 *
 * It creates (cmd_count - 1) pipes and forks (cmd_count) children.
 * Each child reads from the previous pipe and writes to the next pipe.
 */
void	execute_bonus_pipex(char *file1, char **commands, char *file2, int cmd_count)
{
	int		**pipes;         // Array of pipe file descriptors
	int		infile_fd;       // Input file descriptor
	int		outfile_fd;      // Output file descriptor
	pid_t	*pids;           // Array of process IDs
	int		i;
	
	/* Step 1: Open input file */
	infile_fd = open_file(file1, O_RDONLY, 0);
	if (infile_fd == -1)
		return;
	
	/* Step 2: Create output file */
	outfile_fd = open_file(file2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outfile_fd == -1)
	{
		close_fd(infile_fd);
		return;
	}
	
	/* Step 3: Allocate memory for pipes and process IDs */
	pipes = malloc((cmd_count - 1) * sizeof(int *));
	if (!pipes)
	{
		handle_error("malloc failed for pipes");
		close_fd(infile_fd);
		close_fd(outfile_fd);
		return;
	}
	
	pids = malloc(cmd_count * sizeof(pid_t));
	if (!pids)
	{
		handle_error("malloc failed for pids");
		close_fd(infile_fd);
		close_fd(outfile_fd);
		free(pipes);
		return;
	}
	
	/* Step 4: Create all pipes */
	for (i = 0; i < cmd_count - 1; i++)
	{
		pipes[i] = malloc(2 * sizeof(int));
		if (!pipes[i] || pipe(pipes[i]) == -1)
		{
			handle_error("pipe creation failed");
			/* Clean up already created pipes */
			for (int j = 0; j < i; j++)
				free(pipes[j]);
			free(pipes);
			free(pids);
			close_fd(infile_fd);
			close_fd(outfile_fd);
			return;
		}
	}
	
	/* Step 5: Create child processes for each command */
	for (i = 0; i < cmd_count; i++)
	{
		pids[i] = fork();
		if (pids[i] == -1)
		{
			handle_error("fork failed");
			/* Clean up and exit */
			for (int j = 0; j < cmd_count - 1; j++)
			{
				close_fd(pipes[j][0]);
				close_fd(pipes[j][1]);
				free(pipes[j]);
			}
			free(pipes);
			free(pids);
			close_fd(infile_fd);
			close_fd(outfile_fd);
			return;
		}
		
		if (pids[i] == 0)  // Child process
		{
			/* Setup stdin for first command */
			if (i == 0)
			{
				if (dup2(infile_fd, STDIN_FILENO) == -1)
				{
					handle_error("dup2 failed for child stdin");
					exit(1);
				}
			}
			else
			{
				/* Read from previous pipe */
				if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
				{
					handle_error("dup2 failed for child stdin");
					exit(1);
				}
			}
			
			/* Setup stdout for last command */
			if (i == cmd_count - 1)
			{
				if (dup2(outfile_fd, STDOUT_FILENO) == -1)
				{
					handle_error("dup2 failed for child stdout");
					exit(1);
				}
			}
			else
			{
				/* Write to next pipe */
				if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
				{
					handle_error("dup2 failed for child stdout");
					exit(1);
				}
			}
			
			/* Close all pipe file descriptors */
			for (int j = 0; j < cmd_count - 1; j++)
			{
				close_fd(pipes[j][0]);
				close_fd(pipes[j][1]);
			}
			close_fd(infile_fd);
			close_fd(outfile_fd);
			
			/* Execute command */
			execve((char *)commands[i][0], commands[i], NULL);
			
			/* If execve returns, command failed */
			handle_error("execve failed");
			exit(1);
		}
	}
	
	/* Step 6: Parent process - close all file descriptors and wait for children */
	for (int j = 0; j < cmd_count - 1; j++)
	{
		close_fd(pipes[j][0]);
		close_fd(pipes[j][1]);
		free(pipes[j]);
	}
	free(pipes);
	
	close_fd(infile_fd);
	close_fd(outfile_fd);
	free(pids);
	
	/* Wait for all children to complete */
	for (i = 0; i < cmd_count; i++)
	{
		waitpid(pids[i], NULL, 0);
	}
}