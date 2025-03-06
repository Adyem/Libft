#include "libft.hpp"

void ft_to_lower(char *string)
{
	while (string && *string)
	{
		if (*string >= 'a' && *string <= 'z')
			*string += 32;
		string++;
	}
	return ;
}
