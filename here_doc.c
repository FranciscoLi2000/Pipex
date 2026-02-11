/* here_doc.c - Here document functionality for bonus part */
/* Author: your_login */
/* Description: Implements here_document support for pipex */

#include "pipex.h"

/**
 * read_here_doc - Read input from stdin until limiter is found
 * @limiter: String that marks end of here document
 * @pipefd: Pipe file descriptor to write to
 *
 * This function reads lines from standard input until it finds a line
 * containing only the limiter string. It writes all lines to the pipe.
 * This simulates the behavior of: cat << LIMITER | cmd1 | cmd2 ...
 */
void	read_here_doc(char *limiter, int pipefd)
{
	char	*line;
	size_t	limiter_len;
	
	limiter_len = ft_strlen(limiter);
	
	ft_printf("Enter here document (end with: %s): ", limiter);
	
	while (1)
	{
		line = ft_calloc(BUFFER_SIZE + 1, 1);
		if (!line)
		{
			handle_error("malloc failed for here_doc line");
			exit(1);
		}
		
		/* Read a line from stdin */
		if (read(STDIN_FILENO, line, BUFFER_SIZE) <= 0)
		{
			free(line);
			break;
		}
		
		/* Remove newline character */
		size_t line_len = ft_strlen(line);
		if (line_len > 0 && line[line_len - 1] == '\n')
			line[line_len - 1] = '\0';
		
		/* Check if we've reached the limiter */
		if (line_len == limiter_len && ft_strcmp(line, limiter) == 0)
		{
			free(line);
			break;
		}
		
		/* Write line to pipe, adding newline back */
		if (line_len > 0)
		{
			line[line_len] = '\n';
			if (write(pipefd, line, line_len + 1) == -1)
			{
				handle_error("write failed for here_doc");
				free(line);
				break;
			}
		}
		free(line);
	}
}

/**
 * execute_here_doc - Execute pipex with here document input
 * @limiter: Delimiter for here document
 * @commands: Array of command arrays
 * @file: Output file path
 * @cmd_count: Number of commands
 *
 * This function implements: cat << LIMITER | cmd1 | cmd2 | ... | cmdn > file
 *
 * It:
 * 1. Creates pipes for the command chain
 * 2. Reads from here_doc and feeds to first command
 * 3. Chains commands together
 * 4. Writes output to file
 */
void	execute_here_doc(char *limiter, char **commands, char *file, int cmd_count)
{
	int		**pipes;         // Array of pipe file descriptors
	int		infile_pipe[2];  // Pipe for here_doc input
	int		outfile_fd;      // Output file descriptor
	pid_t	*pids;           // Array of process IDs
	int		i;
	
	/* Step 1: Create output file */
	outfile_fd = open_file(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outfile_fd == -1)
		return;
	
	/* Step 2: Create pipe for here_doc input */
	if (pipe(infile_pipe) == -1)
	{
		handle_error("pipe creation failed for here_doc");
		close_fd(outfile_fd);
		return;
	}
	
	/* Step 3: Allocate memory for command pipes */
	pipes = malloc((cmd_count - 1) * sizeof(int *));
	if (!pipes)
	{
		handle_error("malloc failed for pipes");
		close_fd(infile_pipe[0]);
		close_fd(infile_pipe[1]);
		close_fd(outfile_fd);
		return;
	}
	
	/* Step 4: Create all pipes for command chain */
	for (i = 0; i < cmd_count - 1; i++)
	{
		pipes[i] = malloc(2 * sizeof(int));
		if (!pipes[i] || pipe(pipes[i]) == -1)
		{
			handle_error("pipe creation failed");
			/* Clean up already created pipes */
			for (int j = 0; j < i; j++)
			{
				close_fd(pipes[j][0]);
				close_fd(pipes[j][1]);
				free(pipes[j]);
			}
			free(pipes);
			close_fd(infile_pipe[0]);
			close_fd(infile_pipe[1]);
			close_fd(outfile_fd);
			return;
		}
	}
	
	/* Step 5: Allocate process IDs */
	pids = malloc(cmd_count * sizeof(pid_t));
	if (!pids)
	{
		handle_error("malloc failed for pids");
		/* Clean up */
		for (int j = 0; j < cmd_count - 1; j++)
		{
			close_fd(pipes[j][0]);
			close_fd(pipes[j][1]);
			free(pipes[j]);
		}
		free(pipes);
		close_fd(infile_pipe[0]);
		close_fd(infile_pipe[1]);
		close_fd(outfile_fd);
		return;
	}
	
	/* Step 6: Create child processes for each command */
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
			close_fd(infile_pipe[0]);
			close_fd(infile_pipe[1]);
			close_fd(outfile_fd);
			free(pids);
			return;
		}
		
		if (pids[i] == 0)  // Child process
		{
			/* Setup stdin for first command */
			if (i == 0)
			{
				if (dup2(infile_pipe[0], STDIN_FILENO) == -1)
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
			close_fd(infile_pipe[0]);
			close_fd(infile_pipe[1]);
			for (int j = 0; j < cmd_count - 1; j++)
			{
				close_fd(pipes[j][0]);
				close_fd(pipes[j][1]);
			}
			close_fd(outfile_fd);
			
			/* Execute command */
			char	*cmd_path = find_command_path(commands[i][0]);
					if (!cmd_path)
					{
						handle_error("command not found");
						exit(1);
					}
					commands[i][0] = cmd_path;
					execve(commands[i][0], commands[i], NULL);
			
			/* If execve returns, command failed */
			handle_error("execve failed");
			exit(1);
		}
	}
	
	/* Step 7: Parent process - read from here_doc */
	close_fd(infile_pipe[0]);  // Close read end, we'll write to it
	read_here_doc(limiter, infile_pipe[1]);
	close_fd(infile_pipe[1]);  // Close write end after writing
	
	/* Step 8: Close all pipe file descriptors */
	for (int j = 0; j < cmd_count - 1; j++)
	{
		close_fd(pipes[j][0]);
		close_fd(pipes[j][1]);
		free(pipes[j]);
	}
	free(pipes);
	close_fd(outfile_fd);
	free(pids);
	
	/* Step 9: Wait for all children to complete */
	for (i = 0; i < cmd_count; i++)
	{
		waitpid(pids[i], NULL, 0);
	}
}