#include "libft.h"

static size_t	count_words(char const *s, char c)
{
	size_t	cnt;
	size_t	i;

	cnt = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c && (i == 0 || s[i - 1] == c))
			cnt++;
		i++;
	}
	return (cnt);
}

static char	**free_all(char **arr)
{
	size_t	i;

	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
	return (NULL);
}

static size_t	word_len(char const *s, size_t i, char c)
{
	size_t	len;

	len = 0;
	while (s[i + len] && s[i + len] != c)
		len++;
	return (len);
}

char	**ft_split(char const *s, char c)
{
	char	**res;
	size_t	i;
	size_t	j;
	size_t	len;

	if (!s)
		return (NULL);
	res = malloc(sizeof(char *) * (count_words(s, c) + 1));
	if (!res)
		return (NULL);
	i = 0;
	j = 0;
	while (j < count_words(s, c))
	{
		while (s[i] == c)
			i++;
		len = word_len(s, i, c);
		res[j] = ft_substr(s, i, len);
		if (!res[j])
			return (free_all(res));
		i += len;
		j++;
	}
	res[j] = NULL;
	return (res);
}
