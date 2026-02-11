/* string_utils.c - Basic string utility functions */
/* Author: your_login */
/* Description: Basic string manipulation functions to replace libft */

#include "pipex.h"

/**
 * ft_strcmp - Compare two strings
 * @s1: First string
 * @s2: Second string
 *
 * Return: Integer less than, equal to, or greater than zero if s1 is
 *         respectively less than, equal to, or greater than s2
 */
int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;
	
	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * ft_strlen - Calculate the length of a string
 * @s: String to measure
 *
 * Return: Length of string s (excluding null terminator)
 */
size_t	ft_strlen(const char *s)
{
	size_t	length;
	
	length = 0;
	while (s[length])
		length++;
	
	return (length);
}

/**
 * ft_strlcpy - Copy a string (similar to BSD strlcpy)
 * @dst: Destination string
 * @src: Source string
 * @size: Size of destination buffer
 *
 * Return: Total length of src (length of string being copied)
 */
size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	src_len;
	size_t	i;
	
	src_len = ft_strlen(src);
	
	if (size == 0)
		return (src_len);
	
	i = 0;
	while (i < size - 1 && src[i])
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	
	return (src_len);
}

/**
 * ft_printf - Simple printf implementation
 * @format: Format string
 * @...: Variable arguments
 *
 * Return: Number of characters printed
 *
 * This is a simplified version of printf that handles basic format specifiers
 */
int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		printed;
	
	va_start(args, format);
	printed = vprintf(format, args);
	va_end(args);
	
	return (printed);
}

/**
 * ft_strchr - Locate character in string
 * @s: String to search
 * @c: Character to find
 *
 * Return: Pointer to first occurrence of c in s, or NULL if not found
 */
char	*ft_strchr(const char *s, int c)
{
	size_t	i;
	
	i = 0;
	while (s[i])
	{
		if ((unsigned char)s[i] == (unsigned char)c)
			return ((char *)&s[i]);
		i++;
	}
	
	/* Check for the null terminator */
	if ((unsigned char)c == '\0')
		return ((char *)&s[i]);
	
	return (NULL);
}

/**
 * ft_strdup - Duplicate a string
 * @s: String to duplicate
 *
 * Return: Pointer to newly allocated string, or NULL on failure
 */
char	*ft_strdup(const char *s)
{
	size_t	len;
	char	*result;
	
	if (!s)
		return (NULL);
	
	len = ft_strlen(s) + 1; /* +1 for null terminator */
	result = malloc(len);
	if (!result)
		return (NULL);
	
	ft_strlcpy(result, s, len);
	return (result);
}

/**
 * ft_calloc - Allocate and zero-initialize memory
 * @count: Number of elements
 * @size: Size of each element
 *
 * Return: Pointer to allocated memory, or NULL on failure
 */
void	*ft_calloc(size_t count, size_t size)
{
	void	*ptr;
	size_t	total;
	
	total = count * size;
	ptr = malloc(total);
	if (!ptr)
		return (NULL);
	
	/* Zero the memory */
	ft_memset(ptr, 0, total);
	return (ptr);
}

/**
 * ft_memset - Fill memory with a constant byte
 * @s: Memory area
 * @c: Byte value
 * @n: Number of bytes
 *
 * Return: Pointer to memory area s
 */
void	*ft_memset(void *s, int c, size_t n)
{
	size_t	i;
	unsigned char	*p;
	
	p = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		p[i] = (unsigned char)c;
		i++;
	}
	return (s);
}