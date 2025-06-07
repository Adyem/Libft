#include "libft.hpp"

void ft_to_lower(char *string)
{
	while (string && *string)
	{
		if (*string >= 'A' && *string <= 'Z')
			*string += 32;
		string++;
	}
	return ;
}
