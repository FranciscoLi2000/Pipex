/* pipex.c - Main program for pipex */
/* Author: your_login */
/* Description: Entry point for pipex program, handles command line arguments */

#include "pipex.h"

/**
 * print_usage - Print usage information
 */
void	print_usage(void)
{
	ft_printf("Usage: ./pipex file1 cmd1 cmd2 file2\n");
	ft_printf("       ./pipex file1 cmd1 cmd2 cmd3 ... cmdn file2 (bonus)\n");
	ft_printf("       ./pipex here_doc LIMITER cmd1 cmd2 ... cmdn file (bonus)\n");
}

/**
 * is_here_doc - Check if first argument is here_doc
 * @arg: First command line argument
 *
 * Return: true if here_doc, false otherwise
 */
bool	is_here_doc(char *arg)
{
	return (ft_strcmp(arg, "here_doc") == 0);
}

/**
 * check_arguments - Validate command line arguments
 * @argc: Number of arguments
 * @argv: Array of arguments
 * @file1: First input file
 * @cmd_count: Number of commands
 *
 * Return: true if arguments are valid, false otherwise
 */
bool	check_arguments(int argc, char **argv, char **file1, int *cmd_count)
{
	if (argc < 5)
	{
		ft_printf("Error: Not enough arguments\n");
		print_usage();
		return (false);
	}
	
	*file1 = argv[1];
	*cmd_count = argc - 4; // file1 + last file + at least 2 commands
	
	return (true);
}

/**
 * main - Entry point of pipex program
 * @argc: Number of command line arguments
 * @argv: Array of command line arguments
 *
 * Return: 0 on success, 1 on error
 */
int	main(int argc, char **argv)
{
	char	*file1;
	int		cmd_count;
	
	// Check arguments
	if (!check_arguments(argc, argv, &file1, &cmd_count))
		return (1);
	
	// Handle here_doc case
	if (is_here_doc(argv[1]))
	{
#ifdef BONUS_FEATURES
		if (argc < 6)
		{
			ft_printf("Error: here_doc requires at least 3 commands\n");
			return (1);
		}
		
		// Parse commands for here_doc: argv[2] = limiter, argv[3...] = commands, argv[argc-1] = file
		char	*limiter = argv[2];
		char	**commands = &argv[3];
		char	*file2 = argv[argc - 1];
		
		execute_here_doc(limiter, commands, file2, argc - 4);
#else
		ft_printf("Error: here_doc functionality requires bonus compilation\n");
		return (1);
#endif
	}
	// Handle multiple pipes (bonus)
	else if (cmd_count > 2)
	{
#ifdef BONUS_FEATURES
		// Parse: argv[1] = file1, argv[2..argc-2] = commands, argv[argc-1] = file2
		char	*file2 = argv[argc - 1];
		char	**commands = &argv[2];
		
		execute_bonus_pipex(file1, commands, file2, cmd_count);
#else
		ft_printf("Error: Multiple pipes require bonus compilation\n");
		return (1);
#endif
	}
	// Handle basic pipe (mandatory)
	else
	{
		// Parse: argv[1] = file1, argv[2] = cmd1, argv[3] = cmd2, argv[4] = file2
		char	*file2 = argv[4];
		char	**cmd1 = split_command(argv[2]);
		char	**cmd2 = split_command(argv[3]);
		
		if (!cmd1 || !cmd2)
		{
			ft_printf("Error: Failed to parse commands\n");
			if (cmd1)
				cleanup_command(cmd1);
			return (1);
		}
		
		execute_pipex(file1, cmd1, cmd2, file2);
		
		cleanup_command(cmd1);
		cleanup_command(cmd2);
	}
	
	return (0);
}