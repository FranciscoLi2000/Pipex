#include "pipex_bonus.h"

static char	*read_line(int fd)
{
	char	*line;
	char	c;
	int		i;

	line = malloc(4096);
	if (!line)
		return (NULL);
	i = 0;
	while (i < 4095 && read(fd, &c, 1) > 0)
	{
		line[i++] = c;
		if (c == '\n')
			break ;
	}
	if (i == 0)
	{
		free(line);
		return (NULL);
	}
	line[i] = 0;
	return (line);
}

static int	is_limiter(char *line, char *limiter, size_t len)
{
	if (!line)
		return (1);
	if (ft_strncmp(line, limiter, len) == 0
		&& (line[len] == '\n' || line[len] == '\0'))
		return (1);
	return (0);
}

void	here_doc(char *limiter)
{
	int		fd;
	char	*line;
	size_t	len;

	len = ft_strlen(limiter);
	fd = open(HEREDOC_TMP, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
		err_exit("here_doc: open");
	while (1)
	{
		ft_putstr_fd("> ", 1);
		line = read_line(0);
		if (is_limiter(line, limiter, len))
		{
			free(line);
			break ;
		}
		ft_putstr_fd(line, fd);
		free(line);
	}
	close(fd);
}
