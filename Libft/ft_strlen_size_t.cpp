#include "libft.hpp"
#include <cstddef>

size_t ft_strlen_size_t(const char *string)
{
	if (!string)
		return (0);
	size_t index = 0;
	while (string[index])
		index++;
	return (index);
}
