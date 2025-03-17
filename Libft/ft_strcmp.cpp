#include "libft.hpp"

int	ft_strcmp(const char *string1, const char *string2)
{
	if (!string1 || !string2)
		return (-1);
	while (*string1 && (*string1) == (*string2))
	{
		string1++;
		string2++;
	}
	return static_cast<int>((*string1) - static_cast<int>(*string2));
}
