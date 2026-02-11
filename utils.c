/* utils.c - Utility functions for pipex */
/* Author: your_login */
/* Description: Helper functions for error handling, file operations, and command parsing */

#include "pipex.h"

/**
 * handle_error - Print error message and exit with error code
 * @message: Error message to display
 *
 * This function prints the error message using perror() for system errors
 * or ft_printf() for custom messages, then exits with failure status.
 */
void	handle_error(char *message)
{
	perror(message);
}

/**
 * open_file - Open a file with specified flags
 * @filename: Path to the file
 * @flags: File opening flags (O_RDONLY, O_WRONLY, etc.)
 * @mode: File permissions (used when creating file)
 *
 * Return: File descriptor on success, -1 on error
 *
 * This function opens a file and handles potential errors.
 * For write operations, it creates the file if it doesn't exist.
 */
int	open_file(char *filename, int flags, mode_t mode)
{
	int	fd;
	
	/* Create file if writing and it doesn't exist */
	if (flags & O_WRONLY || flags & O_RDWR)
	{
		flags |= O_CREAT;
	}
	
	fd = open(filename, flags, mode);
	if (fd == -1)
	{
		handle_error("open file failed");
	}
	
	return (fd);
}

/**
 * close_fd - Safely close a file descriptor
 * @fd: File descriptor to close
 *
 * This function safely closes a file descriptor, ignoring errors
 * for already-closed or invalid file descriptors.
 */
void	close_fd(int fd)
{
	if (fd > 0)
	{
		close(fd);
	}
}

/**
 * count_words - Count number of words in a string
 * @str: String to count words in
 *
 * Return: Number of words
 *
 * A word is defined as a sequence of non-whitespace characters.
 */
static int	count_words(char *str)
{
	int		count;
	bool	in_word;
	
	count = 0;
	in_word = false;
	while (*str)
	{
		if (*str == ' ' || *str == '\t' || *str == '\n')
		{
			in_word = false;
		}
		else if (!in_word)
		{
			count++;
			in_word = true;
		}
		str++;
	}
	
	return (count);
}


/**
 * split_command - Split a command string into arguments
 * @cmd: Command string to split
 *
 * Return: NULL-terminated array of strings (arguments)
 *
 * This function splits a command string into separate arguments
 * similar to shell's command parsing. It handles spaces and tabs
 * as argument separators.
 */
char	**split_command(char *cmd)
{
	char	**result;
	int		word_count;
	int		i;
	int		j;
	int		start;
	
	if (!cmd || *cmd == '\0')
		return (NULL);
	
	/* Count words to allocate memory */
	word_count = count_words(cmd);
	result = malloc((word_count + 1) * sizeof(char *));
	if (!result)
		return (NULL);
	
	/* Initialize result array */
	for (i = 0; i <= word_count; i++)
		result[i] = NULL;
	
	i = 0;
	start = 0;
	while (cmd[start])
	{
		/* Skip whitespace */
		while (cmd[start] == ' ' || cmd[start] == '\t' || cmd[start] == '\n')
			start++;
		
		if (!cmd[start])
			break;
		
		/* Find end of word */
		j = start;
		while (cmd[j] && cmd[j] != ' ' && cmd[j] != '\t' && cmd[j] != '\n')
			j++;
		
		/* Allocate and copy word */
		result[i] = malloc(j - start + 1);
		if (!result[i])
		{
			cleanup_command(result);
			return (NULL);
		}
		
		ft_strlcpy(result[i], cmd + start, j - start + 1);
		i++;
		start = j;
	}
	
	result[i] = NULL;  /* Null-terminate the array */
	
	return (result);
}

/**
 * cleanup_command - Free allocated command array
 * @cmd: Command array to free
 *
 * This function frees all memory allocated for a command array.
 */
void	cleanup_command(char **cmd)
{
	int	i;
	
	if (!cmd)
		return;
	
	for (i = 0; cmd[i]; i++)
	{
		if (cmd[i])
			free(cmd[i]);
	}
	free(cmd);
}

/**
 * find_command_path - Find the full path to a command
 * @cmd: Command name to find
 *
 * Return: Full path to command, or NULL if not found
 *
 * This function searches for the command in PATH environment variable.
 * It tries the command as-is first (in case it's already a full path),
 * then searches through each directory in PATH.
 */
char	*find_command_path(char *cmd)
{
	char	*path_env;
	char	*dir;
	char	*full_path;
	int		cmd_len;
	int		dir_len;
	int		full_len;
	
	if (!cmd || *cmd == '\0')
		return (NULL);
	
	/* If cmd contains '/', treat it as a full path */
	if (ft_strchr(cmd, '/'))
	{
		/* Check if the file exists and is executable */
		if (access(cmd, X_OK) == 0)
		{
			full_path = ft_strdup(cmd);
			return (full_path);
		}
		return (NULL);
	}
	
	/* Get PATH environment variable */
	path_env = getenv("PATH");
	if (!path_env)
		return (NULL);
	
	cmd_len = ft_strlen(cmd);
	
	/* Split PATH by ':' and search each directory */
	dir = path_env;
	while (*dir)
	{
		/* Find end of current directory */
		dir_len = 0;
		while (dir[dir_len] && dir[dir_len] != ':')
			dir_len++;
		
		/* Build full path: directory/cmd */
		full_len = dir_len + cmd_len + 2; // +2 for '/' and '\0'
		full_path = malloc(full_len);
		if (!full_path)
			continue;
		
		ft_strlcpy(full_path, dir, dir_len + 1);
		full_path[dir_len] = '/';
		ft_strlcpy(full_path + dir_len + 1, cmd, cmd_len + 1);
		
		/* Check if the file exists and is executable */
		if (access(full_path, X_OK) == 0)
			return (full_path);
		
		/* Free and try next directory */
		free(full_path);
		
		/* Move to next directory */
		if (dir[dir_len] == ':')
			dir_len++;
		dir += dir_len;
	}
	
	return (NULL);
}