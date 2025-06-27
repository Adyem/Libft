#include "libft.hpp"

int ft_strlen(const char *string)
{
	if (!string)
        return (0);
	int index = 0;
	while (string[index])
		index++;
	return (index);
}
