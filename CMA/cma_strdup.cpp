#include <stddef.h>
#include <stdbool.h>
#include "CMA.hpp"

char	*cma_strdup(const char *string)
{
	int		len;
	char	*new_string;
	int		i;

	if (!string)
		return (NULL);
	len = 0;
	while (string[len])
		len++;
	len++;
	new_string = static_cast<char *>(cma_malloc(len));
	if (!new_string)
		return (NULL);
	i = 0;
	while (i < len)
	{
		new_string[i] = string[i];
		i++;
	}
	return (new_string);
}
