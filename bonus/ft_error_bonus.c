#include "pipex_bonus.h"

void	err_exit(char *msg)
{
	perror(msg);
	unlink(HEREDOC_TMP);
	exit(EXIT_FAILURE);
}

void	free_tab(char **tab)
{
	int	i;

	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}
