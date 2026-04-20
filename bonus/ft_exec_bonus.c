#include "pipex_bonus.h"

void	exec_cmd(char *arg, char **envp)
{
	char	**cmd;
	char	*path;

	cmd = ft_split(arg, ' ');
	if (!cmd || !cmd[0])
	{
		free_tab(cmd);
		err_exit(ERR_CMD);
	}
	path = get_path(cmd[0], envp);
	if (!path)
	{
		free_tab(cmd);
		err_exit(ERR_CMD);
	}
	execve(path, cmd, envp);
	free_tab(cmd);
	free(path);
	err_exit(ERR_EXEC);
}
