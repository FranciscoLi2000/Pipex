/* cleanup.c - Resource cleanup functions */
/* Author: your_login */
/* Description: Functions to clean up resources and handle cleanup on errors */

#include "pipex.h"

/**
 * cleanup_resources - Clean up all opened file descriptors
 * @pipefd: Array containing pipe file descriptors
 * @infile_fd: Input file descriptor
 * @outfile_fd: Output file descriptor
 *
 * This function safely closes all file descriptors that might have been
 * opened during the execution. It's called in case of error to prevent
 * file descriptor leaks.
 */
void	cleanup_resources(int *pipefd, int *infile_fd, int *outfile_fd)
{
	if (pipefd)
	{
		close_fd(pipefd[0]);
		close_fd(pipefd[1]);
	}
	
	if (infile_fd)
	{
		close_fd(*infile_fd);
		*infile_fd = -1;
	}
	
	if (outfile_fd)
	{
		close_fd(*outfile_fd);
		*outfile_fd = -1;
	}
}