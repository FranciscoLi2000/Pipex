#include "pipex.h"

static char	*check_paths(char **dirs, char *cmd)
{
	char	*temp;
	char	*path;
	int		i;

	i = 0;
	while (dirs[i])
	{
		temp = ft_strjoin(dirs[i], "/");
		path = ft_strjoin(temp, cmd);
		free(temp);
		if (access(path, F_OK | X_OK) == 0)
		{
			free_tab(dirs);
			return (path);
		}
		free(path);
		i++;
	}
	free_tab(dirs);
	return (NULL);
}

char	*get_path(char *cmd, char **envp)
{
	char	**path_env;
	int		i;

	i = 0;
	while (envp[i] && ft_strncmp(envp[i], "PATH=", 5))
		i++;
	if (!envp[i])
		return (NULL);
	path_env = ft_split(envp[i] + 5, ':');
	if (!path_env)
		return (NULL);
	return (check_paths(path_env, cmd));
}
