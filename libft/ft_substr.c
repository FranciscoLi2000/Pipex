#include "libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char		*sub;
	size_t		slen;
	size_t		i;

	if (!s)
		return (NULL);
	slen = ft_strlen(s);
	if (start >= (unsigned int)slen)
		len = 0;
	else if (len > slen - start)
		len = slen - start;
	sub = malloc(len + 1);
	if (!sub)
		return (NULL);
	i = 0;
	while (i < len)
	{
		sub[i] = s[start + i];
		i++;
	}
	sub[i] = 0;
	return (sub);
}
